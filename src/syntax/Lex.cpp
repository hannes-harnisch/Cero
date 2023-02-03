#include "cero/syntax/Lex.hpp"

#include "cero/driver/Message.hpp"
#include "cero/syntax/Token.hpp"
#include "syntax/Encoding.hpp"

namespace cero
{

namespace
{
	Token identify_word_lexeme(std::string_view lexeme)
	{
		struct Keyword
		{
			std::string_view lexeme;
			Token			 kind;
		};
		using enum Token;
		static constexpr Keyword KEYWORDS[] {
			{"break", Break},	{"catch", Catch},	{"const", Const},	{"continue", Continue},
			{"else", Else},		{"enum", Enum},		{"for", For},		{"if", If},
			{"in", In},			{"let", Let},		{"public", Public}, {"return", Return},
			{"static", Static}, {"struct", Struct}, {"switch", Switch}, {"throw", Throw},
			{"try", Try},		{"use", Use},		{"var", Var},		{"while", While},
		};

		for (auto& keyword : KEYWORDS)
			if (lexeme == keyword.lexeme)
				return keyword.kind;

		return Name;
	}
} // namespace

class Lexer
{
	const Source&		   source;
	Reporter&			   reporter;
	Source::Iterator	   cursor;
	const Source::Iterator source_begin;
	const Source::Iterator source_end;

public:
	Lexer(const Source& source, Reporter& reporter) :
		source(source),
		reporter(reporter),
		cursor(source.begin()),
		source_begin(cursor),
		source_end(source.end())
	{}

	TokenStream lex()
	{
		TokenStream stream;

		size_t source_length = source.get_text().length();
		if (source_length > LexicalToken::MAX_LENGTH)
			report(Message::SourceInputTooLarge, cursor, LexicalToken::MAX_LENGTH);
		else
		{
			while (cursor != source_end)
				next_token(stream);
		}

		stream.tokens.emplace_back(Token::EndOfFile, 0, static_cast<uint32_t>(source_length));
		return stream;
	}

private:
	void next_token(TokenStream& stream)
	{
		auto token_begin = cursor;

		Token kind;

		char current = *cursor++;
		switch (current)
		{
			using enum Token;
			case ' ':
			case '\t':
			case '\v':
			case '\f':
			case '\r': return;
			case '\n': kind = NewLine; break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': kind = lex_number(current); break;
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
			case 'Z': kind = lex_word(); break;
			case '.': kind = match_dot(); break;
			case ':': kind = match_colon(); break;
			case ',': kind = Comma; break;
			case ';': kind = Semicolon; break;
			case '{': kind = LeftBrace; break;
			case '}': kind = RightBrace; break;
			case '(': kind = LeftParen; break;
			case ')': kind = RightParen; break;
			case '[': kind = LeftBracket; break;
			case ']': kind = RightBracket; break;
			case '<': kind = match_left_angle(); break;
			case '>': kind = match_right_angle(); break;
			case '=': kind = match_equal(); break;
			case '+': kind = match_plus(); break;
			case '-': kind = match_minus(); break;
			case '*': kind = match_star(); break;
			case '/': kind = match_slash(); break;
			case '%': kind = match_percent(); break;
			case '!': kind = match_bang(); break;
			case '&': kind = match_ampersand(); break;
			case '|': kind = match_pipe(); break;
			case '~': kind = match_tilde(); break;
			case '^': kind = Caret; break;
			case '?': kind = QuestionMark; break;
			case '@': kind = At; break;
			case '$': kind = Dollar; break;
			case '#': kind = Hash; break;
			case '"':
			{
				eat_quoted_sequence('"');
				kind = StringLiteral;
				break;
			}
			case '\'':
			{
				eat_quoted_sequence('\'');
				kind = CharLiteral;
				break;
			}
			default:
			{
				eat_unicode_token(current);
				kind = Name;
				break;
			}
		}

		auto length = static_cast<uint32_t>(cursor - token_begin);
		auto offset = static_cast<uint32_t>(token_begin - source_begin);
		stream.tokens.emplace_back(kind, length, offset);
	}

	bool match(char expected)
	{
		if (cursor == source_end)
			return false;

		if (*cursor != expected)
			return false;

		++cursor;
		return true;
	}

	Token match_dot()
	{
		if (match('.'))
		{
			if (match('.'))
				return Token::Ellipsis;

			--cursor; // step back to ensure the extra dot is not skipped
		}
		else if (is_dec_digit(*cursor))
		{
			eat_number_literal(is_dec_digit);
			return Token::FloatLiteral;
		}

		return Token::Dot;
	}

	Token lex_number(char first)
	{
		if (first == '0' && cursor != source_end)
		{
			switch (*cursor)
			{
				case 'x':
					++cursor;
					eat_number_literal(is_hex_digit);
					return Token::HexIntLiteral;
				case 'b':
					++cursor;
					eat_number_literal(is_dec_digit);
					return Token::BinIntLiteral;
				case 'o':
					++cursor;
					eat_number_literal(is_dec_digit);
					return Token::OctIntLiteral;
			}
		}

		eat_number_literal(is_dec_digit);

		if (cursor != source_end && *cursor == '.')
		{
			auto cursor_at_dot = cursor;
			++cursor;

			if (eat_decimal_number())
				return Token::FloatLiteral;
			else
				cursor = cursor_at_dot;
		}
		return Token::DecIntLiteral;
	}

	void eat_number_literal(bool (*char_predicate)(char))
	{
		while (cursor != source_end)
		{
			char it = *cursor;

			if (!char_predicate(it) && it != ' ' && it != '\t')
				break;

			++cursor;
		}
	}

	bool eat_decimal_number()
	{
		bool ate = false;
		while (cursor != source_end)
		{
			char it = *cursor;

			if (is_dec_digit(it))
				ate = true;
			else if (it != ' ' && it != '\t')
				break;

			++cursor;
		}
		return ate;
	}

	Token match_colon()
	{
		if (match(':'))
			return Token::ColonColon;

		return Token::Colon;
	}

	Token match_left_angle()
	{
		if (match('<'))
		{
			if (match('='))
				return Token::LeftAngleAngleEqual;

			return Token::LeftAngleAngle;
		}
		if (match('='))
			return Token::LeftAngleEqual;

		return Token::LeftAngle;
	}

	Token match_right_angle()
	{
		if (match('>'))
		{
			if (match('='))
				return Token::RightAngleAngleEqual;

			return Token::RightAngleAngle;
		}
		if (match('='))
			return Token::RightAngleEqual;

		return Token::RightAngle;
	}

	Token match_equal()
	{
		if (match('='))
			return Token::EqualEqual;
		if (match('>'))
			return Token::ThickArrow;

		return Token::Equal;
	}

	Token match_plus()
	{
		if (match('+'))
			return Token::PlusPlus;
		if (match('='))
			return Token::PlusEqual;

		return Token::Plus;
	}

	Token match_minus()
	{
		if (match('>'))
			return Token::ThinArrow;
		if (match('-'))
			return Token::MinusMinus;
		if (match('='))
			return Token::MinusEqual;

		return Token::Minus;
	}

	Token match_star()
	{
		if (match('*'))
		{
			if (match('='))
				return Token::StarStarEqual;

			return Token::StarStar;
		}
		if (match('='))
			return Token::StarEqual;

		return Token::Star;
	}

	Token match_slash()
	{
		if (match('/'))
		{
			eat_line_comment();
			return Token::LineComment;
		}
		if (match('*'))
		{
			eat_block_comment();
			return Token::BlockComment;
		}
		if (match('='))
			return Token::SlashEqual;

		return Token::Slash;
	}

	void eat_line_comment()
	{
		while (cursor != source_end)
		{
			if (*cursor == '\n')
				break;

			++cursor;
		}
	}

	void eat_block_comment()
	{
		auto comment_begin = cursor;

		uint32_t unclosed_count = 1;
		while (cursor != source_end)
		{
			if (match('*'))
			{
				if (match('/') && --unclosed_count == 0)
					return;
			}
			else if (match('/'))
			{
				if (match('*'))
					++unclosed_count;
			}
			else
				++cursor;
		}

		if (unclosed_count != 0)
			report(Message::UnterminatedBlockComment, comment_begin);
	}

	Token match_percent()
	{
		if (match('='))
			return Token::PercentEqual;

		return Token::Percent;
	}

	Token match_bang()
	{
		if (match('='))
			return Token::BangEqual;

		return Token::Bang;
	}

	Token match_ampersand()
	{
		if (match('&'))
			return Token::DoubleAmpersand;
		if (match('='))
			return Token::AmpersandEqual;

		return Token::Ampersand;
	}

	Token match_pipe()
	{
		if (match('|'))
			return Token::PipePipe;
		if (match('='))
			return Token::PipeEqual;

		return Token::Pipe;
	}

	Token match_tilde()
	{
		if (match('='))
			return Token::TildeEqual;

		return Token::Tilde;
	}

	void eat_quoted_sequence(char quote)
	{
		bool ignore_quote = false;
		while (cursor != source_end)
		{
			char it = *cursor;
			if (it == '\n')
			{
				report(Message::MissingClosingQuote, cursor);
				break;
			}

			++cursor;

			if (it == '\\')
				ignore_quote ^= true; // bool gets flipped so we correctly handle an escaped backslash within the literal
			else if (it == quote && !ignore_quote)
				break;
			else if (ignore_quote)
				ignore_quote = false;
		}
	}

	Token lex_word()
	{
		auto word_begin = cursor - 1;
		eat_word_token_rest();

		std::string_view lexeme(word_begin, cursor);
		return identify_word_lexeme(lexeme);
	}

	void eat_unicode_token(char first)
	{
		if (check_multibyte_utf8_value(first, is_utf8_xid_start))
			eat_word_token_rest();
	}

	void eat_word_token_rest()
	{
		while (cursor != source_end)
		{
			char it = *cursor;
			if (is_standard_ascii(it))
			{
				if (!is_ascii_word_character(it))
					break;
			}
			else
			{
				if (!check_multibyte_utf8_value(it, is_utf8_xid_continue))
					break;
			}
			++cursor;
		}
	}

	bool check_multibyte_utf8_value(char first, bool (*utf8_predicate)(uint32_t))
	{
		const auto leading_byte = static_cast<uint8_t>(first);
		const auto leading_ones = static_cast<uint32_t>(std::countl_one(leading_byte));

		uint32_t encoding = leading_byte;

		bool valid = false;
		if (leading_ones >= 2 && leading_ones <= 4)
		{
			const uint32_t bytes_to_read = leading_ones - 1;

			uint8_t* encoding_byte_ptr = reinterpret_cast<uint8_t*>(&encoding) + 1;
			for (uint32_t i = 0; i != bytes_to_read && cursor != source_end; ++i)
				*encoding_byte_ptr++ = static_cast<uint8_t>(*cursor++);

			valid = utf8_predicate(encoding);
		}

		if (!valid)
			report(Message::UnexpectedCharacter, cursor - 1, encoding);

		return valid;
	}

	template<typename... Args>
	void report(CheckedMessage<Args...> message, Source::Iterator report_cursor, Args&&... args) const
	{
		reporter.report(message, source.locate(report_cursor), std::forward<Args>(args)...);
	}
};

TokenStream lex(const Source& source, Reporter& reporter)
{
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero
