#include "Lexer.hpp"

#include "driver/Message.hpp"
#include "syntax/CharUtils.hpp"

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
		if (source.get_text().length() > Token::MAX_LENGTH)
			report<Message::SourceInputTooLarge>(cursor);
		else
		{
			while (cursor != end)
				next_token();
		}

		tokens.append({TokenKind::EndOfFile, 0, 0});
		return std::move(tokens);
	}

private:
	void next_token()
	{
		auto token_begin = cursor;

		TokenKind t;
		switch (*cursor++)
		{
			using enum TokenKind;
			case ' ':
			case '\t': return;
			case '\n':
			case '\v':
			case '\f':
			case '\r': t = NewLine; break;
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
			case '_':
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z': t = lex_word(); break;
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
			case '\\':
				++token_begin;
				t = lex_escaped_keyword();
				break;
			default: t = lex_unicode(); break;
		}

		uint32_t length = static_cast<uint32_t>(cursor - token_begin);
		uint32_t offset = static_cast<uint32_t>(token_begin - begin);
		tokens.append({t, length, offset});
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

	TokenKind match_dot()
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

	TokenKind lex_number()
	{
		char first = cursor[-1];
		if (first == '.')
		{
			eat_number_literal<is_dec_digit>();
			return TokenKind::DecFloatLiteral;
		}

		if (first == '0' && cursor != end)
		{
			switch (*cursor)
			{
				case 'x':
					++cursor;
					eat_number_literal<is_hex_digit>();
					return TokenKind::HexIntLiteral;
				case 'b':
					++cursor;
					eat_number_literal<is_bin_digit>();
					return TokenKind::BinIntLiteral;
				case 'o':
					++cursor;
					eat_number_literal<is_oct_digit>();
					return TokenKind::OctIntLiteral;
			}
		}

		auto kind = TokenKind::DecIntLiteral;
		while (cursor != end)
		{
			char it = *cursor;
			if (it == '.')
				kind = TokenKind::DecFloatLiteral; // TODO: maybe refactor this so it's cleaner
			else if (!is_dec_digit(it) && !is_non_breaking_whitespace(it))
				break;

			++cursor;
		}
		return kind;
	}

	template<bool (*CHAR_PREDICATE)(char)>
	void eat_number_literal()
	{
		while (cursor != end)
		{
			char it = *cursor;
			if (!CHAR_PREDICATE(it) && !is_non_breaking_whitespace(it))
				break;

			++cursor;
		}
	}

	TokenKind match_colon()
	{
		if (match(':'))
			return TokenKind::ColonColon;

		return TokenKind::Colon;
	}

	TokenKind match_left_bracket()
	{
		if (match('^'))
		{
			if (match(']'))
				return TokenKind::BracketedCaret;

			--cursor; // step back to ensure caret is not skipped
		}
		return TokenKind::LeftBracket;
	}

	TokenKind match_left_angle()
	{
		if (match('<'))
		{
			if (match('='))
				return TokenKind::DoubleLeftAngleEqual;

			return TokenKind::DoubleLeftAngle;
		}
		if (match('='))
		{
			if (match('>'))
				return TokenKind::Spaceship;

			return TokenKind::LessEqual;
		}
		return TokenKind::LeftAngle;
	}

	TokenKind match_right_angle()
	{
		if (match('>'))
		{
			if (match('='))
				return TokenKind::DoubleRightAngleEqual;

			return TokenKind::DoubleRightAngle;
		}
		if (match('='))
			return TokenKind::GreaterEqual;

		return TokenKind::RightAngle;
	}

	TokenKind match_equal()
	{
		if (match('='))
			return TokenKind::EqualEqual;
		if (match('>'))
			return TokenKind::ThickArrow;

		return TokenKind::Equal;
	}

	TokenKind match_plus()
	{
		if (match('+'))
			return TokenKind::PlusPlus;
		if (match('='))
			return TokenKind::PlusEqual;

		return TokenKind::Plus;
	}

	TokenKind match_minus()
	{
		if (match('>'))
			return TokenKind::ThinArrow;
		if (match('-'))
			return TokenKind::MinusMinus;
		if (match('='))
			return TokenKind::MinusEqual;

		return TokenKind::Minus;
	}

	TokenKind match_star()
	{
		if (match('*'))
		{
			if (match('='))
				return TokenKind::StarStarEqual;

			return TokenKind::StarStar;
		}
		if (match('='))
			return TokenKind::StarEqual;

		return TokenKind::Star;
	}

	TokenKind match_slash()
	{
		if (match('/'))
			return lex_line_comment();
		if (match('!'))
			to_do(); // doc comments
		if (match('*'))
			to_do(); // block comments
		if (match('='))
			return TokenKind::SlashEqual;

		return TokenKind::Slash;
	}

	TokenKind lex_line_comment()
	{
		while (cursor != end)
		{
			if (is_breaking_whitespace(*cursor))
				break;

			++cursor;
		}
		return TokenKind::LineComment;
	}

	TokenKind match_percent()
	{
		if (match('='))
			return TokenKind::PercentEqual;

		return TokenKind::Percent;
	}

	TokenKind match_bang()
	{
		if (match('='))
			return TokenKind::BangEqual;

		return TokenKind::Bang;
	}

	TokenKind match_ampersand()
	{
		if (match('&'))
			return TokenKind::DoubleAmpersand;
		if (match('='))
			return TokenKind::AmpersandEqual;

		return TokenKind::Ampersand;
	}

	TokenKind match_pipe()
	{
		if (match('|'))
			return TokenKind::PipePipe;
		if (match('='))
			return TokenKind::PipeEqual;

		return TokenKind::Pipe;
	}

	TokenKind match_tilde()
	{
		if (match('='))
			return TokenKind::TildeEqual;

		return TokenKind::Tilde;
	}

	TokenKind lex_string()
	{
		eat_quoted_sequence('"');
		return TokenKind::StringLiteral;
	}

	TokenKind lex_character()
	{
		eat_quoted_sequence('\'');
		return TokenKind::CharLiteral;
	}

	void eat_quoted_sequence(char quote)
	{
		bool ignore_quote = false;
		while (cursor != end)
		{
			char it = *cursor;
			if (is_breaking_whitespace(it))
			{
				report<Message::MissingClosingQuote>(cursor);
				break;
			}

			++cursor;

			if (it == '\\')
				ignore_quote ^= true;
			else if (it == quote && !ignore_quote)
				break;
			else if (ignore_quote)
				ignore_quote = false;
		}
	}

	TokenKind lex_word()
	{
		auto word_begin = cursor - 1;
		while (cursor != end)
		{
			if (!is_word_char(*cursor))
				break;

			++cursor;
		}

		std::string_view lexeme(word_begin, cursor);
		return identify_word_lexeme(lexeme);
	}

	TokenKind lex_escaped_keyword()
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

		return TokenKind::Name;
	}

	TokenKind lex_unicode()
	{
		char first_char = cursor[-1];
		if (can_begin_names(first_char)) // TODO: use xid-start
			return lex_word();

		report<Message::IllegalChar>(cursor - 1, static_cast<int>(first_char));
		return TokenKind::Name; // treat unknown character as name for better parsing
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
