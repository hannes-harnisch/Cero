#include "Lexer.hpp"
#include "syntax/CharUtils.hpp"
#include "util/Enum.hpp"
#include "util/LookupTable.hpp"
#include "util/StringMap.hpp"

constexpr LookupTable<TokenKind, uint16_t> compute_token_length_lookup_table()
{
	LookupTable<TokenKind, uint16_t> table;

	using enum TokenKind;
	static_assert(std::to_underlying(Dot) == 7);
	static_assert(std::to_underlying(ThinArrow) == 35);
	static_assert(std::to_underlying(Spaceship) == 57);
	static_assert(std::to_underlying(EndOfFile) == 98,
				  "After a change to the TokenKind enum, ensure that the lookup table computation is still correct.");

	auto increment = [](TokenKind& kind) {
		kind = static_cast<TokenKind>(std::to_underlying(kind) + 1);
	};
	for (auto i = Dot; i != ThinArrow; increment(i))
		table[i] = 1;

	for (auto i = ThinArrow; i != Spaceship; increment(i))
		table[i] = 2;

	for (auto i = Spaceship; i != As; increment(i))
		table[i] = 3;

	for (auto i = As; i != EndOfFile; increment(i))
		table[i] = static_cast<uint16_t>(magic_enum::enum_name(i).length());

	return table;
}

constexpr LookupTable<TokenKind, uint16_t> TOKEN_LENGTHS = compute_token_length_lookup_table();

static const StringMap<TokenKind> KEYWORDS {
	{"as", TokenKind::As},
	{"async", TokenKind::Async},
	{"await", TokenKind::Await},
	{"break", TokenKind::Break},
	{"catch", TokenKind::Catch},
	{"const", TokenKind::Const},
	{"continue", TokenKind::Continue},
	{"do", TokenKind::Do},
	{"else", TokenKind::Else},
	{"enum", TokenKind::Enum},
	{"extern", TokenKind::Extern},
	{"for", TokenKind::For},
	{"if", TokenKind::If},
	{"in", TokenKind::In},
	{"let", TokenKind::Let},
	{"out", TokenKind::Out},
	{"override", TokenKind::Override},
	{"private", TokenKind::Private},
	{"protected", TokenKind::Protected},
	{"public", TokenKind::Public},
	{"raw", TokenKind::Raw},
	{"return", TokenKind::Return},
	{"sealed", TokenKind::Sealed},
	{"static", TokenKind::Static},
	{"struct", TokenKind::Struct},
	{"super", TokenKind::Super},
	{"switch", TokenKind::Switch},
	{"throw", TokenKind::Throw},
	{"trait", TokenKind::Trait},
	{"try", TokenKind::Try},
	{"use", TokenKind::Use},
	{"var", TokenKind::Var},
	{"virtual", TokenKind::Virtual},
	{"while", TokenKind::While},
	{"yield", TokenKind::Yield},
};

class Lexer
{
	TokenStream			   tokens;
	const Source&		   source;
	Reporter&			   reporter;
	Source::Iterator	   cursor;
	const Source::Iterator begin;
	const Source::Iterator end;

public:
	Lexer(const Source& source, Reporter& reporter) :
		source(source),
		reporter(reporter),
		cursor(source.begin()),
		begin(cursor),
		end(source.end())
	{}

	TokenStream lex()
	{
		while (cursor != end)
			next_token();

		tokens.append({TokenKind::EndOfFile, 0, 0});
		return std::move(tokens);
	}

private:
	struct UnplacedToken
	{
		TokenKind kind;
		uint16_t  length;

		UnplacedToken() = default;

		UnplacedToken(TokenKind kind) :
			kind(kind),
			length(TOKEN_LENGTHS[kind])
		{}

		UnplacedToken(TokenKind kind, uint16_t length) :
			kind(kind),
			length(length)
		{}
	};

	void next_token()
	{
		uint32_t offset = static_cast<uint32_t>(cursor - begin);

		UnplacedToken t;
		switch (*cursor++)
		{
			using enum TokenKind;
			case ' ':
			case '\t':
			case '\r': return;
			case '\n': t = NewLine; break;
			case '.': t = match_dot(); break;
			case ':': t = match_colon(); break;
			case ',': t = Comma; break;
			case ';': t = Semicolon; break;
			case '{': t = LeftBrace; break;
			case '}': t = RightBrace; break;
			case '(': t = LeftParen; break;
			case ')': t = RightParen; break;
			case '[': t = match_left_bracket(); break;
			case ']': t = RightBracket; break;
			case '<': t = match_left_angle(); break;
			case '>': t = match_right_angle(); break;
			case '=': t = match_equal(); break;
			case '+': t = match_plus(); break;
			case '-': t = match_minus(); break;
			case '*': t = match_star(); break;
			case '/': t = match_slash(); break;
			case '%': t = match_percent(); break;
			case '!': t = match_bang(); break;
			case '&': t = match_ampersand(); break;
			case '|': t = match_pipe(); break;
			case '~': t = match_tilde(); break;
			case '^': t = Caret; break;
			case '?': t = QuestionMark; break;
			case '@': t = At; break;
			case '$': t = Dollar; break;
			case '#': t = Hash; break;
			case '"': t = lex_string(); break;
			case '\'': t = lex_character(); break;
			case '\\': t = lex_escaped_keyword(); break;
			default: t = lex_alphanumeric(); break;
		}
		tokens.append({t.kind, t.length, offset});
	}

	bool match(char expected)
	{
		if (cursor == end)
			return false;

		if (*cursor != expected)
			return false;

		++cursor;
		return true;
	}

	UnplacedToken match_dot()
	{
		if (match('.'))
		{
			if (match('.'))
				return TokenKind::Ellipsis;

			--cursor;
		}
		else if (is_dec_digit(*cursor))
			return lex_number();

		return TokenKind::Dot;
	}

	UnplacedToken lex_number()
	{
		auto num_begin = cursor - 1;
		auto kind	   = determine_number_literal_kind();

		auto length = cursor - num_begin;
		if (length > UINT16_MAX)
			report<Message::TokenTooLong>(num_begin);

		return {kind, static_cast<uint16_t>(length)};
	}

	TokenKind determine_number_literal_kind()
	{
		char first = cursor[-1];
		if (first == '.')
		{
			eat_number_literal<is_dec_digit>();
			return TokenKind::Rational;
		}
		else if (first == '0' && cursor != end)
		{
			switch (*cursor)
			{
				case 'x': eat_number_literal<is_hex_digit>(); return TokenKind::Integer;
				case 'b': eat_number_literal<is_bin_digit>(); return TokenKind::Integer;
				case 'o': eat_number_literal<is_oct_digit>(); return TokenKind::Integer;
			}
		}

		auto kind = TokenKind::Integer;
		while (cursor != end)
		{
			char ch = *cursor++;
			if (ch == '.')
				kind = TokenKind::Rational; // TODO incorrect

			if (!is_dec_digit(ch) && !is_ignored_whitespace(ch))
				break;
		}
		return kind;
	}

	template<bool (*CHAR_PREDICATE)(char)>
	void eat_number_literal()
	{
		while (cursor != end)
		{
			char ch = *cursor++;
			if (!CHAR_PREDICATE(ch) && !is_ignored_whitespace(ch))
				break;
		}
	}

	UnplacedToken match_colon()
	{
		if (match(':'))
			return TokenKind::ColonColon;

		return TokenKind::Colon;
	}

	UnplacedToken match_left_bracket()
	{
		if (match('^'))
		{
			if (match(']'))
				return TokenKind::BracketedCaret;

			--cursor;
		}
		return TokenKind::LeftBracket;
	}

	UnplacedToken match_left_angle()
	{
		if (match('<'))
		{
			if (match('='))
				return TokenKind::TwoLeftAngleEqual;

			return TokenKind::TwoLeftAngles;
		}
		else if (match('='))
		{
			if (match('>'))
				return TokenKind::Spaceship;

			return TokenKind::LessEqual;
		}
		return TokenKind::LeftAngle;
	}

	UnplacedToken match_right_angle()
	{
		if (match('>'))
		{
			if (match('='))
				return TokenKind::TwoRightAngleEqual;

			return TokenKind::TwoRightAngles;
		}
		else if (match('='))
			return TokenKind::GreaterEqual;

		return TokenKind::RightAngle;
	}

	UnplacedToken match_equal()
	{
		if (match('='))
			return TokenKind::EqualEqual;
		else if (match('>'))
			return TokenKind::ThickArrow;

		return TokenKind::Equal;
	}

	UnplacedToken match_plus()
	{
		if (match('+'))
			return TokenKind::PlusPlus;
		else if (match('='))
			return TokenKind::PlusEqual;

		return TokenKind::Plus;
	}

	UnplacedToken match_minus()
	{
		if (match('>'))
			return TokenKind::ThinArrow;
		else if (match('-'))
			return TokenKind::MinusMinus;
		else if (match('='))
			return TokenKind::MinusEqual;

		return TokenKind::Minus;
	}

	UnplacedToken match_star()
	{
		if (match('*'))
		{
			if (match('='))
				return TokenKind::StarStarEqual;

			return TokenKind::StarStar;
		}
		else if (match('='))
			return TokenKind::StarEqual;

		return TokenKind::Star;
	}

	UnplacedToken match_slash()
	{
		if (match('/'))
			return lex_comment();
		else if (match('='))
			return TokenKind::SlashEqual;

		return TokenKind::Slash;
	}

	UnplacedToken lex_comment()
	{
		auto comment_begin = cursor - 1;
		while (cursor != end)
		{
			if (*cursor++ == '\n') // TODO emit newline token after comment?
				break;
		}

		auto length = cursor - comment_begin;
		if (length > UINT16_MAX)
			report<Message::TokenTooLong>(comment_begin);

		return {TokenKind::Comment, static_cast<uint16_t>(length)};
	}

	UnplacedToken match_percent()
	{
		if (match('='))
			return TokenKind::PercentEqual;

		return TokenKind::Percent;
	}

	UnplacedToken match_bang()
	{
		if (match('='))
			return TokenKind::BangEqual;

		return TokenKind::Bang;
	}

	UnplacedToken match_ampersand()
	{
		if (match('&'))
			return TokenKind::TwoAmpersands;
		else if (match('='))
			return TokenKind::AmpersandEqual;

		return TokenKind::Ampersand;
	}

	UnplacedToken match_pipe()
	{
		if (match('|'))
			return TokenKind::PipePipe;
		else if (match('='))
			return TokenKind::PipeEqual;

		return TokenKind::Pipe;
	}

	UnplacedToken match_tilde()
	{
		if (match('='))
			return TokenKind::TildeEqual;

		return TokenKind::Tilde;
	}

	UnplacedToken lex_string()
	{
		return lex_quoted_sequence('"', TokenKind::String);
	}

	UnplacedToken lex_character()
	{
		return lex_quoted_sequence('\'', TokenKind::Character);
	}

	UnplacedToken lex_quoted_sequence(char quote, TokenKind kind)
	{
		auto sequence_begin = cursor - 1;

		bool ignore_quote = false;
		while (cursor != end)
		{
			char ch = *cursor++;

			if (ch == '\\') // TODO: check
				ignore_quote ^= true;

			if (ch == quote && !ignore_quote)
				break;

			if (ignore_quote)
				ignore_quote = false;

			if (ch == '\n')
			{
				report<Message::MissingClosingQuote>(--cursor);
				break;
			}
		}

		auto length = cursor - sequence_begin + 1;
		if (length > UINT16_MAX)
			report<Message::TokenTooLong>(sequence_begin);

		return {kind, static_cast<uint16_t>(length)};
	}

	UnplacedToken lex_escaped_keyword()
	{
		auto name_begin = cursor;
		while (cursor != end)
		{
			if (!is_word_char(*cursor))
				break;

			++cursor;
		}

		std::string_view kw(name_begin, cursor);
		if (KEYWORDS.find(kw) == KEYWORDS.end())
			report<Message::EscapedNonKeyword>(name_begin, kw);

		return {TokenKind::Name, static_cast<uint16_t>(kw.length())};
	}

	UnplacedToken lex_alphanumeric()
	{
		char ch = cursor[-1];
		if (can_begin_names(ch))
			return lex_word();
		else if (is_dec_digit(ch))
			return lex_number();

		report<Message::IllegalChar>(cursor - 1, static_cast<int>(cursor[-1]));
		return {TokenKind::Name, 1}; // treat unknown character as name for better parsing
	}

	UnplacedToken lex_word()
	{
		auto name_begin = cursor - 1;
		while (cursor != end)
		{
			if (!is_word_char(*cursor))
				break;

			++cursor;
		}

		std::string_view kw(name_begin, cursor);

		auto it	  = KEYWORDS.find(kw);
		auto kind = it == KEYWORDS.end() ? TokenKind::Name : it->second;

		auto length = cursor - name_begin;
		if (length > UINT16_MAX)
			report<Message::TokenTooLong>(name_begin);

		return {kind, static_cast<uint16_t>(length)};
	}

	template<Message MESSAGE, typename... Ts>
	void report(Source::Iterator cursor, Ts&&... ts) const
	{
		reporter.report<MESSAGE>(source.locate(cursor), std::forward<Ts>(ts)...);
	}
};

TokenStream lex(const Source& source, Reporter& reporter)
{
	Lexer lexer(source, reporter);
	return lexer.lex();
}
