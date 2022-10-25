#include "Lexer.hpp"

#include "driver/Message.hpp"
#include "syntax/CharUtils.hpp"
#include "util/Enum.hpp"
#include "util/LookupTable.hpp"

constexpr LookupTable<TokenKind, uint32_t> compute_token_length_lookup_table()
{
	LookupTable<TokenKind, uint32_t> table;

	for (auto value : magic_enum::enum_values<TokenKind>())
		table[value] = get_token_length(value);

	return table;
}

constexpr LookupTable<TokenKind, uint32_t> TOKEN_LENGTHS = compute_token_length_lookup_table();

TokenKind identify_word_lexeme(std::string_view lexeme)
{
	struct Keyword
	{
		std::string_view word;
		TokenKind		 token_kind;
	};
	using enum TokenKind;
	static constexpr Keyword KEYWORDS[] {
		{"as", As},
		{"async", Async},
		{"await", Await},
		{"break", Break},
		{"catch", Catch},
		{"const", Const},
		{"continue", Continue},
		{"do", Do},
		{"else", Else},
		{"enum", Enum},
		{"for", For},
		{"if", If},
		{"in", In},
		{"let", Let},
		{"out", Out},
		{"override", Override},
		{"private", Private},
		{"protected", Protected},
		{"public", Public},
		{"raw", Raw},
		{"return", Return},
		{"sealed", Sealed},
		{"static", Static},
		{"struct", Struct},
		{"super", Super},
		{"switch", Switch},
		{"throw", Throw},
		{"trait", Trait},
		{"try", Try},
		{"use", Use},
		{"var", Var},
		{"virtual", Virtual},
		{"while", While},
		{"yield", Yield},
	};

	for (auto& keyword : KEYWORDS)
		if (lexeme == keyword.word)
			return keyword.token_kind;

	return Name;
}

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
		TokenKind kind : Token::KIND_BITS;
		uint32_t  length : Token::LENGTH_BITS;

		UnplacedToken() = default;

		UnplacedToken(TokenKind kind) :
			kind(kind),
			length(TOKEN_LENGTHS[kind])
		{}

		UnplacedToken(TokenKind kind, uint32_t length) :
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
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': t = lex_number(); break;
			default: t = lex_alphabetic(); break;
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

			--cursor; // step back to ensure the extra dot is not skipped
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
		if (length > Token::MAX_LENGTH)
			report<Message::TokenTooLong>(cursor);

		return {kind, static_cast<uint32_t>(length)};
	}

	TokenKind determine_number_literal_kind()
	{
		char first = cursor[-1];
		if (first == '.')
		{
			eat_number_literal<is_dec_digit>();
			return TokenKind::DecFloatLiteral;
		}
		else if (first == '0' && cursor != end)
		{
			switch (*cursor)
			{
				case 'x': eat_number_literal<is_hex_digit>(); return TokenKind::HexIntLiteral;
				case 'b': eat_number_literal<is_bin_digit>(); return TokenKind::BinIntLiteral;
				case 'o': eat_number_literal<is_oct_digit>(); return TokenKind::OctIntLiteral;
			}
		}

		auto kind = TokenKind::DecIntLiteral;
		while (cursor != end)
		{
			char ch = *cursor++;
			if (ch == '.')
				kind = TokenKind::DecFloatLiteral; // TODO incorrect

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

			--cursor; // step back to ensure caret is not skipped
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
			return lex_line_comment();
		else if (match('='))
			return TokenKind::SlashEqual;

		return TokenKind::Slash;
	}

	UnplacedToken lex_line_comment()
	{
		auto comment_begin = cursor;
		while (cursor != end)
		{
			if (*cursor == '\n')
				break;

			++cursor;
		}

		auto length = cursor - comment_begin;
		if (length > Token::MAX_LENGTH)
			report<Message::TokenTooLong>(cursor);

		return {TokenKind::LineComment, static_cast<unsigned>(length)};
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
		return {TokenKind::StringLiteral, lex_quoted_sequence('"')};
	}

	UnplacedToken lex_character()
	{
		return {TokenKind::CharLiteral, lex_quoted_sequence('\'')};
	}

	uint32_t lex_quoted_sequence(char quote)
	{
		auto sequence_begin = cursor - 1;

		bool ignore_quote = false;
		while (cursor != end)
		{
			char ch = *cursor++;

			if (ch == '\\')
				ignore_quote ^= true;
			else if (ch == quote && !ignore_quote)
				break;
			else if (ignore_quote)
				ignore_quote = false;

			if (ch == '\n')
			{
				report<Message::MissingClosingQuote>(--cursor);
				break;
			}
		}

		auto length = cursor - sequence_begin;
		if (length > Token::MAX_LENGTH)
			report<Message::TokenTooLong>(cursor);

		return static_cast<uint32_t>(length);
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

		std::string_view lexeme(name_begin, cursor);

		auto kind = identify_word_lexeme(lexeme);
		if (kind == TokenKind::Name)
			report<Message::EscapedNonKeyword>(name_begin, lexeme);

		return {TokenKind::Name, static_cast<uint32_t>(lexeme.length())};
	}

	UnplacedToken lex_alphabetic()
	{
		char first_char = cursor[-1];
		if (can_begin_names(first_char))
			return lex_word();

		report<Message::IllegalChar>(cursor - 1, static_cast<int>(first_char));
		return {TokenKind::Name, 1}; // treat unknown character as name for better parsing
	}

	UnplacedToken lex_word()
	{
		auto word_begin = cursor - 1;
		while (cursor != end)
		{
			if (!is_word_char(*cursor))
				break;

			++cursor;
		}

		auto length = cursor - word_begin;
		if (length > Token::MAX_LENGTH)
			report<Message::TokenTooLong>(cursor);

		std::string_view lexeme(word_begin, cursor);

		auto kind = identify_word_lexeme(lexeme);
		return {kind, static_cast<uint32_t>(length)};
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
