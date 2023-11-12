#include "Lex.hpp"

#include "cero/io/Message.hpp"
#include "cero/syntax/Encoding.hpp"
#include "cero/syntax/LexCursor.hpp"

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

	TokenKind identify_keyword(std::string_view lexeme) {
		using enum TokenKind;

		if (lexeme.length() > sizeof(uint64_t)) {
			return Name;
		}

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
	Lexer(const SourceLock& source, Reporter& reporter) :
		source(source),
		reporter(reporter),
		text(source.get_text()),
		cursor(text) {
	}

	TokenStream lex() {
		TokenStream stream;

		if (text.length() > MaxSourceLength) {
			report(Message::SourceInputTooLarge, cursor, MaxSourceLength);
		} else {
			while (auto next = cursor.peek()) {
				next_token(*next, stream);
			}
		}

		stream.tokens.push_back({{TokenKind::EndOfFile, static_cast<uint32_t>(text.length())}, 0});
		return stream;
	}

private:
	const SourceLock& source;
	Reporter& reporter;
	std::string_view text;
	LexCursor cursor;

	void next_token(char next, TokenStream& stream) {
		const auto token_begin = cursor;
		cursor.advance();

		using enum TokenKind;
		TokenKind kind;
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

		const uint32_t rest = token_begin.count_rest();
		const uint32_t offset = static_cast<uint32_t>(text.length()) - rest;
		const uint32_t length = rest - cursor.count_rest();
		stream.tokens.push_back({{kind, offset}, length});
	}

	TokenKind lex_word(LexCursor token_begin) {
		eat_word_token_rest();
		auto length = token_begin.count_rest() - cursor.count_rest();
		auto lexeme = token_begin.rest().substr(0, length);
		return identify_keyword(lexeme);
	}

	void eat_word_token_rest() {
		while (auto next = cursor.peek()) {
			const char c = *next;
			if (is_standard_ascii(c)) {
				if (!is_ascii_word_character(c)) {
					break;
				}
			} else {
				if (!check_multibyte_utf8_value(c, is_utf8_xid_continue, cursor)) {
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
				*bytes++ = cursor.next().value_or('\0');
			}

			valid = utf8_predicate(encoding);
		}

		if (!valid) {
			report(Message::UnexpectedCharacter, token_begin, encoding);
		}
		return valid;
	}

	TokenKind lex_number(char next) {
		if (next == '0') {
			auto backup = cursor;
			switch (cursor.next().value_or('\0')) {
				case 'x': eat_number_literal(is_hex_digit); return TokenKind::HexIntLiteral;
				case 'b': eat_number_literal(is_dec_digit); return TokenKind::BinIntLiteral;
				case 'o': eat_number_literal(is_dec_digit); return TokenKind::OctIntLiteral;
			}
			cursor = backup;
		}

		eat_number_literal(is_dec_digit);
		auto cursor_at_token_end = cursor;

		while (auto c = cursor.peek()) {
			if (!is_whitespace(*c)) {
				break;
			}
			cursor.advance();
		}

		auto cursor_at_dot = cursor;

		// check for rational part
		next = cursor.next().value_or('\0');
		if (next == '.') {
			if (eat_decimal_number()) {
				return TokenKind::FloatLiteral;
			} else {
				cursor = cursor_at_dot; // reset to dot if there's no fractional part
			}
		} else {
			cursor = cursor_at_token_end;
		}

		return TokenKind::DecIntLiteral;
	}

	void eat_number_literal(bool (*char_predicate)(char)) {
		auto lookahead = cursor;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (char_predicate(c)) {
				cursor = lookahead;
				cursor.advance();
			} else if (!is_whitespace(c)) {
				break;
			}

			lookahead.advance();
		}
	}

	bool eat_decimal_number() {
		bool matched = false;
		auto lookahead = cursor;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (is_dec_digit(c)) {
				cursor = lookahead;
				cursor.advance();
				matched = true;
			} else if (!is_whitespace(c)) {
				break;
			}

			lookahead.advance();
		}
		return matched;
	}

	TokenKind match_dot() {
		auto backup = cursor;
		if (cursor.match('.')) {
			if (cursor.match('.')) {
				return TokenKind::Ellipsis;
			}
			cursor = backup; // reset to ensure the extra dot is not skipped
		} else if (is_dec_digit(cursor.peek().value_or('\0'))) {
			eat_number_literal(is_dec_digit);
			return TokenKind::FloatLiteral;
		}

		return TokenKind::Dot;
	}

	TokenKind match_colon() {
		if (cursor.match(':')) {
			return TokenKind::ColonColon;
		}
		return TokenKind::Colon;
	}

	TokenKind match_left_angle() {
		if (cursor.match('<')) {
			if (cursor.match('=')) {
				return TokenKind::LeftAngleAngleEquals;
			}
			return TokenKind::LeftAngleAngle;
		}

		if (cursor.match('=')) {
			return TokenKind::LeftAngleEquals;
		}

		return TokenKind::LeftAngle;
	}

	TokenKind match_right_angle() {
		auto backup = cursor;
		if (cursor.match('>')) {
			if (cursor.match('=')) {
				return TokenKind::RightAngleAngleEquals;
			}
			cursor = backup; // reset to ensure the extra right angle is not skipped
		} else if (cursor.match('=')) {
			return TokenKind::RightAngleEquals;
		}

		return TokenKind::RightAngle;
	}

	TokenKind match_equal() {
		if (cursor.match('=')) {
			return TokenKind::EqualsEquals;
		}
		if (cursor.match('>')) {
			return TokenKind::ThickArrow;
		}
		return TokenKind::Equals;
	}

	TokenKind match_plus() {
		if (cursor.match('+')) {
			return TokenKind::PlusPlus;
		}
		if (cursor.match('=')) {
			return TokenKind::PlusEquals;
		}
		return TokenKind::Plus;
	}

	TokenKind match_minus() {
		if (cursor.match('>')) {
			return TokenKind::ThinArrow;
		}
		if (cursor.match('-')) {
			return TokenKind::MinusMinus;
		}
		if (cursor.match('=')) {
			return TokenKind::MinusEquals;
		}
		return TokenKind::Minus;
	}

	TokenKind match_star() {
		if (cursor.match('*')) {
			if (cursor.match('=')) {
				return TokenKind::StarStarEquals;
			}
			return TokenKind::StarStar;
		}

		if (cursor.match('=')) {
			return TokenKind::StarEquals;
		}
		return TokenKind::Star;
	}

	TokenKind match_slash() {
		if (cursor.match('/')) {
			eat_line_comment();
			return TokenKind::LineComment;
		}

		if (cursor.match('*')) {
			eat_block_comment();
			return TokenKind::BlockComment;
		}

		if (cursor.match('=')) {
			return TokenKind::SlashEquals;
		}
		return TokenKind::Slash;
	}

	void eat_line_comment() {
		while (auto next = cursor.peek()) {
			if (*next == '\n') {
				break;
			}
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

		if (unclosed_count != 0) {
			report(Message::UnterminatedBlockComment, comment_begin);
		}
	}

	TokenKind match_percent() {
		if (cursor.match('=')) {
			return TokenKind::PercentEquals;
		}
		return TokenKind::Percent;
	}

	TokenKind match_bang() {
		if (cursor.match('=')) {
			return TokenKind::BangEquals;
		}
		return TokenKind::Bang;
	}

	TokenKind match_ampersand() {
		if (cursor.match('&')) {
			return TokenKind::AmpersandAmpersand;
		}
		if (cursor.match('=')) {
			return TokenKind::AmpersandEquals;
		}
		return TokenKind::Ampersand;
	}

	TokenKind match_pipe() {
		if (cursor.match('|')) {
			return TokenKind::PipePipe;
		}
		if (cursor.match('=')) {
			return TokenKind::PipeEquals;
		}
		return TokenKind::Pipe;
	}

	TokenKind match_tilde() {
		if (cursor.match('=')) {
			return TokenKind::TildeEquals;
		}
		return TokenKind::Tilde;
	}

	void eat_quoted_sequence(char quote) {
		bool ignore_quote = false;
		while (auto next = cursor.peek()) {
			const char c = *next;
			if (c == '\n') {
				report(Message::MissingClosingQuote, cursor);
				break;
			}

			cursor.advance();

			if (c == '\\') {
				ignore_quote ^= true; // bool gets flipped so we correctly handle an escaped backslash within the literal
			} else if (c == quote && !ignore_quote) {
				break;
			} else if (ignore_quote) {
				ignore_quote = false;
			}
		}
	}

	void eat_unicode_token(char next, LexCursor token_begin) {
		if (check_multibyte_utf8_value(next, is_utf8_xid_start, token_begin)) {
			eat_word_token_rest();
		}
	}

	template<typename... Args>
	void report(Message message, LexCursor report_cursor, Args&&... args) const {
		auto offset = static_cast<uint32_t>(text.length()) - report_cursor.count_rest();
		auto location = source.locate(offset);
		reporter.report(message, location, std::forward<Args>(args)...);
	}
};

TokenStream lex(const SourceLock& source, Reporter& reporter) {
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero
