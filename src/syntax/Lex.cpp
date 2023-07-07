#include "Lex.hpp"

#include "driver/Message.hpp"
#include "syntax/Encoding.hpp"
#include "syntax/LexCursor.hpp"
#include "syntax/Token.hpp"

namespace cero {

namespace {

	constexpr uint64_t u64_encode(std::string_view lexeme) {
		struct Bytes {
			unsigned char array[sizeof(uint64_t)] = {};
		};

		Bytes bytes;
		std::copy_n(lexeme.begin(), std::min(lexeme.length(), sizeof(uint64_t)), bytes.array);
		return std::bit_cast<uint64_t>(bytes);
	}

	Token identify_keyword(std::string_view lexeme) {
		using enum Token;

		if (lexeme.length() > sizeof(uint64_t))
			return Name;

		switch (u64_encode(lexeme)) {
			case u64_encode("break"): return Break;
			case u64_encode("catch"): return Catch;
			case u64_encode("const"): return Const;
			case u64_encode("continue"): return Continue;
			case u64_encode("do"): return Do;
			case u64_encode("else"): return Else;
			case u64_encode("enum"): return Enum;
			case u64_encode("extern"): return Extern;
			case u64_encode("for"): return For;
			case u64_encode("if"): return If;
			case u64_encode("in"): return In;
			case u64_encode("let"): return Let;
			case u64_encode("private"): return Private;
			case u64_encode("public"): return Public;
			case u64_encode("restrict"): return Restrict;
			case u64_encode("return"): return Return;
			case u64_encode("static"): return Static;
			case u64_encode("struct"): return Struct;
			case u64_encode("switch"): return Switch;
			case u64_encode("throw"): return Throw;
			case u64_encode("try"): return Try;
			case u64_encode("use"): return Use;
			case u64_encode("var"): return Var;
			case u64_encode("while"): return While;
		}
		return Name;
	}

} // namespace

class Lexer {
public:
	Lexer(const Source& source, Reporter& reporter) :
		source(source),
		reporter(reporter),
		text(source.get_text()),
		cursor(source) {
	}

	TokenStream lex() {
		TokenStream stream;

		const uint32_t source_length = static_cast<uint32_t>(text.length());
		if (source_length > LexicalToken::MaxLength) {
			report(Message::SourceInputTooLarge, cursor, LexicalToken::MaxLength);
		} else {
			while (char next = cursor.peek())
				next_token(next, stream);
		}

		stream.tokens.emplace_back(Token::EndOfFile, source_length, 0);
		return stream;
	}

private:
	const Source&	 source;
	Reporter&		 reporter;
	std::string_view text;
	LexCursor		 cursor;

	void next_token(char next, TokenStream& stream) {
		const auto token_begin = cursor;
		cursor.advance();

		using enum Token;
		Token kind;
		switch (next) {
			case ' ':
			case '\t':
			case '\n':
			case '\v':
			case '\f':
			case '\r': return;

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
			case 'Z': kind = lex_word(token_begin); break;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': kind = lex_number(next); break;

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
				eat_quoted_sequence('"');
				kind = StringLiteral;
				break;

			case '\'':
				eat_quoted_sequence('\'');
				kind = CharLiteral;
				break;

			default:
				eat_unicode_token(next, token_begin);
				kind = Name;
				break;
		}

		const uint32_t rest	  = token_begin.count_rest();
		const uint32_t offset = static_cast<uint32_t>(text.length()) - rest;
		const uint32_t length = rest - cursor.count_rest();
		stream.tokens.emplace_back(kind, offset, length);
	}

	Token lex_word(LexCursor token_begin) {
		eat_word_token_rest();
		auto length = token_begin.count_rest() - cursor.count_rest();
		auto lexeme = token_begin.rest().substr(0, length);
		return identify_keyword(lexeme);
	}

	void eat_word_token_rest() {
		while (char next = cursor.peek()) {
			if (is_standard_ascii(next)) {
				if (!is_ascii_word_character(next)) {
					break;
				}
			} else {
				if (!check_multibyte_utf8_value(next, is_utf8_xid_continue, cursor)) {
					break;
				}
			}
			cursor.advance();
		}
	}

	bool check_multibyte_utf8_value(char next, bool (*utf8_predicate)(uint32_t), LexCursor token_begin) {
		const auto leading_byte = static_cast<uint8_t>(next);
		const auto leading_ones = static_cast<uint8_t>(std::countl_one(leading_byte));

		uint32_t encoding = leading_byte;

		bool valid = false;
		if (leading_ones >= 2 && leading_ones <= 4) {
			char* bytes = reinterpret_cast<char*>(&encoding) + 1;

			for (uint32_t i = 1; i != leading_ones; ++i) {
				*bytes++ = cursor.next();
			}

			valid = utf8_predicate(encoding);
		}

		if (!valid)
			report(Message::UnexpectedCharacter, token_begin, encoding);

		return valid;
	}

	Token lex_number(char next) {
		if (next == '0') {
			auto backup = cursor;
			switch (cursor.next()) {
				case 'x': eat_number_literal(is_hex_digit); return Token::HexIntLiteral;
				case 'b': eat_number_literal(is_dec_digit); return Token::BinIntLiteral;
				case 'o': eat_number_literal(is_dec_digit); return Token::OctIntLiteral;
			}
			cursor = backup;
		}

		eat_number_literal(is_dec_digit);
		auto cursor_at_token_end = cursor;

		while (char c = cursor.peek()) {
			if (!is_whitespace(c))
				break;

			cursor.advance();
		}

		auto cursor_at_dot = cursor;

		// check for rational part
		next = cursor.next();
		if (next == '.') {
			if (eat_decimal_number()) {
				return Token::FloatLiteral;
			} else {
				cursor = cursor_at_dot; // reset to dot if there's no fractional part
			}
		} else {
			cursor = cursor_at_token_end;
		}

		return Token::DecIntLiteral;
	}

	void eat_number_literal(bool (*char_predicate)(char)) {
		auto lookahead = cursor;

		while (char next = lookahead.peek()) {
			if (char_predicate(next)) {
				cursor = lookahead;
				cursor.advance();
			} else if (!is_whitespace(next)) {
				break;
			}

			lookahead.advance();
		}
	}

	bool eat_decimal_number() {
		bool matched   = false;
		auto lookahead = cursor;

		while (char next = lookahead.peek()) {
			if (is_dec_digit(next)) {
				cursor = lookahead;
				cursor.advance();
				matched = true;
			} else if (!is_whitespace(next)) {
				break;
			}

			lookahead.advance();
		}
		return matched;
	}

	Token match_dot() {
		auto backup = cursor;
		if (cursor.match('.')) {
			if (cursor.match('.')) {
				return Token::Ellipsis;
			}
			cursor = backup; // reset to ensure the extra dot is not skipped
		} else if (is_dec_digit(cursor.peek())) {
			eat_number_literal(is_dec_digit);
			return Token::FloatLiteral;
		}

		return Token::Dot;
	}

	Token match_colon() {
		if (cursor.match(':'))
			return Token::ColonColon;

		return Token::Colon;
	}

	Token match_left_angle() {
		if (cursor.match('<')) {
			if (cursor.match('='))
				return Token::LeftAngleAngleEquals;

			return Token::LeftAngleAngle;
		}

		if (cursor.match('='))
			return Token::LeftAngleEquals;

		return Token::LeftAngle;
	}

	Token match_right_angle() {
		auto backup = cursor;
		if (cursor.match('>')) {
			if (cursor.match('=')) {
				return Token::RightAngleAngleEquals;
			}
			cursor = backup; // reset to ensure the extra right angle is not skipped
		} else if (cursor.match('=')) {
			return Token::RightAngleEquals;
		}

		return Token::RightAngle;
	}

	Token match_equal() {
		if (cursor.match('='))
			return Token::EqualsEquals;

		if (cursor.match('>'))
			return Token::ThickArrow;

		return Token::Equals;
	}

	Token match_plus() {
		if (cursor.match('+'))
			return Token::PlusPlus;

		if (cursor.match('='))
			return Token::PlusEquals;

		return Token::Plus;
	}

	Token match_minus() {
		if (cursor.match('>'))
			return Token::ThinArrow;

		if (cursor.match('-'))
			return Token::MinusMinus;

		if (cursor.match('='))
			return Token::MinusEquals;

		return Token::Minus;
	}

	Token match_star() {
		if (cursor.match('*')) {
			if (cursor.match('='))
				return Token::StarStarEquals;

			return Token::StarStar;
		}

		if (cursor.match('='))
			return Token::StarEquals;

		return Token::Star;
	}

	Token match_slash() {
		if (cursor.match('/')) {
			eat_line_comment();
			return Token::LineComment;
		}

		if (cursor.match('*')) {
			eat_block_comment();
			return Token::BlockComment;
		}

		if (cursor.match('='))
			return Token::SlashEquals;

		return Token::Slash;
	}

	void eat_line_comment() {
		while (char next = cursor.peek()) {
			if (next == '\n')
				break;

			cursor.advance();
		}
	}

	void eat_block_comment() {
		auto comment_begin = cursor;

		uint32_t unclosed_count = 1;
		while (cursor.valid()) {
			if (cursor.match('*')) {
				if (cursor.match('/') && --unclosed_count == 0) {
					return;
				}
			} else if (cursor.match('/')) {
				if (cursor.match('*')) {
					++unclosed_count;
				}
			} else {
				cursor.advance();
			}
		}

		if (unclosed_count != 0)
			report(Message::UnterminatedBlockComment, comment_begin);
	}

	Token match_percent() {
		if (cursor.match('='))
			return Token::PercentEquals;

		return Token::Percent;
	}

	Token match_bang() {
		if (cursor.match('='))
			return Token::BangEquals;

		return Token::Bang;
	}

	Token match_ampersand() {
		if (cursor.match('&'))
			return Token::AmpersandAmpersand;

		if (cursor.match('='))
			return Token::AmpersandEquals;

		return Token::Ampersand;
	}

	Token match_pipe() {
		if (cursor.match('|'))
			return Token::PipePipe;

		if (cursor.match('='))
			return Token::PipeEquals;

		return Token::Pipe;
	}

	Token match_tilde() {
		if (cursor.match('='))
			return Token::TildeEquals;

		return Token::Tilde;
	}

	void eat_quoted_sequence(char quote) {
		bool ignore_quote = false;
		while (char next = cursor.peek()) {
			if (next == '\n') {
				report(Message::MissingClosingQuote, cursor);
				break;
			}

			cursor.advance();

			if (next == '\\')
				ignore_quote ^= true; // bool gets flipped so we correctly handle an escaped backslash within the literal
			else if (next == quote && !ignore_quote)
				break;
			else if (ignore_quote)
				ignore_quote = false;
		}
	}

	void eat_unicode_token(char next, LexCursor token_begin) {
		if (check_multibyte_utf8_value(next, is_utf8_xid_start, token_begin))
			eat_word_token_rest();
	}

	template<typename... Args>
	void report(Message message, LexCursor report_cursor, Args&&... args) const {
		auto offset	  = static_cast<uint32_t>(text.length()) - report_cursor.count_rest();
		auto location = source.locate(offset);
		reporter.report(message, location, std::forward<Args>(args)...);
	}
};

TokenStream lex(const Source& source, Reporter& reporter) {
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero