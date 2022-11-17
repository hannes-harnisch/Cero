#include "Lexer.hpp"

#include "driver/Message.hpp"
#include "syntax/Encoding.hpp"

namespace
{
	TokenKind identify_word_lexeme(std::string_view lexeme)
	{
		struct Keyword
		{
			std::string_view word;
			TokenKind		 token_kind;
		};
		using enum TokenKind;
		static constexpr Keyword KEYWORDS[] {
			{"break", Break},	{"catch", Catch},	{"const", Const},	{"continue", Continue},
			{"else", Else},		{"enum", Enum},		{"for", For},		{"if", If},
			{"in", In},			{"let", Let},		{"public", Public}, {"return", Return},
			{"static", Static}, {"struct", Struct}, {"switch", Switch}, {"throw", Throw},
			{"try", Try},		{"use", Use},		{"var", Var},		{"while", While},
		};

		for (auto& keyword : KEYWORDS)
			if (lexeme == keyword.word)
				return keyword.token_kind;

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
		TokenStream tokens;

		size_t source_length = source.get_text().length();
		if (source_length > Token::MAX_LENGTH)
			report(Message::SourceInputTooLarge, cursor, Token::MAX_LENGTH);
		else
		{
			while (cursor != source_end)
				next_token(tokens);
		}

		tokens.append({TokenKind::EndOfFile, 0, static_cast<uint32_t>(source_length)});
		return tokens;
	}

private:
	void next_token(TokenStream& tokens)
	{
		auto token_begin = cursor;

		TokenKind kind;

		char current = *cursor++;
		switch (current)
		{
			using enum TokenKind;
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
			case '[': kind = match_left_bracket(); break;
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
			case '\\':
			{
				++token_begin;
				eat_escaped_keyword();
				kind = Name;
				break;
			}
			default:
			{
				eat_unicode_token(current);
				kind = Name;
				break;
			}
		}

		uint32_t length = static_cast<uint32_t>(cursor - token_begin);
		uint32_t offset = static_cast<uint32_t>(token_begin - source_begin);
		tokens.append({kind, length, offset});
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

	TokenKind match_dot()
	{
		if (match('.'))
		{
			if (match('.'))
				return TokenKind::Ellipsis;

			--cursor; // step back to ensure the extra dot is not skipped
		}
		else if (is_dec_digit(*cursor))
		{
			eat_number_literal(is_dec_digit);
			return TokenKind::DecFloatLiteral;
		}

		return TokenKind::Dot;
	}

	TokenKind lex_number(char first)
	{
		// TODO: trim whitespace
		// TODO: disallow `1.`
		if (first == '0' && cursor != source_end)
		{
			switch (*cursor)
			{
				case 'x':
					++cursor;
					eat_number_literal(is_hex_digit);
					return TokenKind::HexIntLiteral;
				case 'b':
					++cursor;
					eat_number_literal(is_dec_digit);
					return TokenKind::BinIntLiteral;
				case 'o':
					++cursor;
					eat_number_literal(is_dec_digit);
					return TokenKind::OctIntLiteral;
			}
		}

		eat_number_literal(is_dec_digit);

		if (cursor != source_end && *cursor == '.')
		{
			++cursor;
			eat_number_literal(is_dec_digit);
			return TokenKind::DecFloatLiteral;
		}

		return TokenKind::DecIntLiteral;
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
			return TokenKind::LeftAngleEqual;

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
			return TokenKind::RightAngleEqual;

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
		{
			eat_line_comment();
			return TokenKind::LineComment;
		}
		if (match('*'))
		{
			eat_block_comment();
			return TokenKind::BlockComment;
		}
		if (match('='))
			return TokenKind::SlashEqual;

		return TokenKind::Slash;
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

		uint32_t nesting_count = 1;
		while (cursor != source_end)
		{
			if (match('*'))
			{
				if (match('/') && --nesting_count == 0)
					return;
			}
			else if (match('/'))
			{
				if (match('*'))
					++nesting_count;
			}
			else
				++cursor;
		}

		if (nesting_count != 0)
			report(Message::UnterminatedBlockComment, comment_begin);
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

	TokenKind lex_word()
	{
		auto word_begin = cursor - 1;
		eat_word_token_rest();

		std::string_view lexeme(word_begin, cursor);
		return identify_word_lexeme(lexeme);
	}

	void eat_escaped_keyword()
	{
		auto name_begin = cursor;

		char it = *cursor;
		if (is_ascii_word_character(it))
		{
			++cursor;
			eat_word_token_rest();
		}
		else if (!is_standard_ascii(it))
		{
			++cursor;
			eat_unicode_token(it);
		}

		std::string_view lexeme(name_begin, cursor);

		auto kind = identify_word_lexeme(lexeme);
		if (kind == TokenKind::Name)
			report(Message::EscapedNonKeyword, name_begin, lexeme);
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
		const uint8_t  leading_byte = static_cast<uint8_t>(first);
		const uint32_t leading_ones = static_cast<uint32_t>(std::countl_one(leading_byte));

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
	void report(CheckedMessage<Args...> message, Source::Iterator cursor, Args&&... args) const
	{
		reporter.report(message, source.locate(cursor), std::forward<Args>(args)...);
	}
};

TokenStream lex(const Source& source, Reporter& reporter)
{
	Lexer lexer(source, reporter);
	return lexer.lex();
}
