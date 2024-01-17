#include "Lex.hpp"

#include "cero/io/Message.hpp"
#include "cero/syntax/Encoding.hpp"
#include "cero/syntax/SourceCursor.hpp"

namespace cero {

namespace {

	TokenKind identify_keyword(std::string_view lexeme) {
		using enum TokenKind;
		switch (lexeme.length()) {
			case 2:
				if (lexeme == "do") return Do;
				if (lexeme == "if") return If;
				if (lexeme == "in") return In;
				break;

			case 3:
				if (lexeme == "for") return For;
				if (lexeme == "let") return Let;
				if (lexeme == "try") return Try;
				if (lexeme == "var") return Var;
				break;

			case 4:
				if (lexeme == "else") return Else;
				if (lexeme == "enum") return Enum;
				break;

			case 5:
				if (lexeme == "break") return Break;
				if (lexeme == "catch") return Catch;
				if (lexeme == "const") return Const;
				if (lexeme == "throw") return Throw;
				if (lexeme == "while") return While;
				break;

			case 6:
				if (lexeme == "public") return Public;
				if (lexeme == "return") return Return;
				if (lexeme == "static") return Static;
				if (lexeme == "struct") return Struct;
				if (lexeme == "switch") return Switch;
				break;

			case 7:
				if (lexeme == "private") return Private;
				break;

			case 8:
				if (lexeme == "continue") return Continue;
				break;

			case 9:
				if (lexeme == "unchecked") return Unchecked;
				break;
		}
		return Name;
	}

} // namespace

class Lexer {
public:
	Lexer(const SourceLock& source, Reporter& reporter) :
		source_(source),
		reporter_(reporter),
		cursor_(source),
		stream_(source) {
	}

	TokenStream lex() {
		if (source_.get_length() > MaxSourceLength) {
			const CodeLocation blank {source_.get_path(), 0, 0};
			reporter_.report(Message::SourceInputTooLarge, blank, MaxSourceLength);
		} else {
			lex_source();
		}

		stream_.add_header({TokenKind::EndOfFile, cursor_.offset()});
		return std::move(stream_);
	}

private:
	const SourceLock& source_;
	Reporter& reporter_;
	SourceCursor cursor_;
	TokenStream stream_;

	void lex_source() {
		while (auto next = cursor_.peek()) {
			const char c = *next;
			const auto begin_offset = cursor_.offset();
			cursor_.advance();

			bool is_variable_length = false;
			TokenKind kind;
			switch (c) {
				case ' ':
				case '\t':
				case '\n':
				case '\v':
				case '\f':
				case '\r': continue;

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
				case 'Z':
					kind = lex_word(begin_offset);
					is_variable_length = kind == TokenKind::Name;
					break;

				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					kind = lex_number(c);
					is_variable_length = true;
					break;

				case '.':
					kind = match_dot();
					is_variable_length = kind == TokenKind::FloatLiteral;
					break;

				case ':': kind = match_colon(); break;
				case ',': kind = TokenKind::Comma; break;
				case ';': kind = TokenKind::Semicolon; break;
				case '{': kind = TokenKind::LeftBrace; break;
				case '}': kind = TokenKind::RightBrace; break;
				case '(': kind = TokenKind::LeftParen; break;
				case ')': kind = TokenKind::RightParen; break;
				case '[': kind = TokenKind::LeftBracket; break;
				case ']': kind = TokenKind::RightBracket; break;
				case '<': kind = match_left_angle(); break;
				case '>': kind = match_right_angle(); break;
				case '=': kind = match_equal(); break;
				case '+': kind = match_plus(); break;
				case '-': kind = match_minus(); break;
				case '*': kind = match_star(); break;
				case '/':
					kind = match_slash();
					is_variable_length = kind == TokenKind::LineComment || kind == TokenKind::BlockComment;
					break;

				case '%': kind = match_percent(); break;
				case '!': kind = match_bang(); break;
				case '&': kind = match_ampersand(); break;
				case '|': kind = match_pipe(); break;
				case '~': kind = match_tilde(); break;
				case '^': kind = TokenKind::Caret; break;
				case '?': kind = TokenKind::QuestionMark; break;
				case '@': kind = TokenKind::At; break;
				case '$': kind = TokenKind::Dollar; break;
				case '#': kind = TokenKind::Hash; break;
				case '"':
					eat_quoted_sequence('"');
					kind = TokenKind::StringLiteral;
					is_variable_length = true;
					break;

				case '\'':
					eat_quoted_sequence('\'');
					kind = TokenKind::CharLiteral;
					is_variable_length = true;
					break;

				default:
					eat_unicode_token(c, begin_offset);
					kind = TokenKind::Name;
					is_variable_length = true;
					break;
			}

			stream_.add_header({kind, begin_offset});
			if (is_variable_length) {
				stream_.add_length(cursor_.offset() - begin_offset);
			}
		}
	}

	TokenKind lex_word(uint32_t begin_offset) {
		eat_word_token_rest();
		auto length = cursor_.offset() - begin_offset;
		auto lexeme = source_.get_text().substr(begin_offset, length);
		return identify_keyword(lexeme);
	}

	void eat_word_token_rest() {
		while (auto next = cursor_.peek()) {
			const char c = *next;
			if (is_standard_ascii(c)) {
				if (!is_ascii_word_character(c)) {
					break;
				}
			} else {
				if (!check_multibyte_utf8_value(c, &is_utf8_xid_continue, cursor_.offset())) {
					break;
				}
			}
			cursor_.advance();
		}
	}

	bool check_multibyte_utf8_value(char next, bool (*utf8_predicate)(uint32_t), uint32_t begin_offset) {
		const auto leading_byte = static_cast<uint8_t>(next);
		const auto leading_ones = static_cast<uint8_t>(std::countl_one(leading_byte));

		uint32_t encoding = leading_byte;

		bool valid = false;
		if (leading_ones >= 2 && leading_ones <= 4) {
			char* bytes = reinterpret_cast<char*>(&encoding) + 1;

			for (uint32_t i = 1; i != leading_ones; ++i) {
				*bytes++ = cursor_.next().value_or('\0');
			}

			valid = utf8_predicate(encoding);
		}

		if (!valid) {
			report(Message::UnexpectedCharacter, begin_offset, encoding);
		}
		return valid;
	}

	TokenKind lex_number(char next) {
		if (next == '0') {
			auto backup = cursor_;
			switch (cursor_.next().value_or('\0')) {
				case 'x': eat_number_literal(&is_hex_digit); return TokenKind::HexIntLiteral;
				case 'b': eat_number_literal(&is_dec_digit); return TokenKind::BinIntLiteral;
				case 'o': eat_number_literal(&is_dec_digit); return TokenKind::OctIntLiteral;
			}
			cursor_ = backup;
		}

		eat_number_literal(&is_dec_digit);
		auto cursor_at_token_end = cursor_;

		while (auto c = cursor_.peek()) {
			if (!is_whitespace(*c)) {
				break;
			}
			cursor_.advance();
		}

		auto cursor_at_dot = cursor_;

		// check for rational part
		next = cursor_.next().value_or('\0');
		if (next == '.') {
			if (eat_decimal_number()) {
				return TokenKind::FloatLiteral;
			} else {
				cursor_ = cursor_at_dot; // reset to dot if there's no fractional part
			}
		} else {
			cursor_ = cursor_at_token_end;
		}

		return TokenKind::DecIntLiteral;
	}

	void eat_number_literal(bool (*char_predicate)(char)) {
		auto lookahead = cursor_;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (char_predicate(c)) {
				cursor_ = lookahead;
				cursor_.advance();
			} else if (!is_whitespace(c)) {
				break;
			}

			lookahead.advance();
		}
	}

	bool eat_decimal_number() {
		bool matched = false;
		auto lookahead = cursor_;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (is_dec_digit(c)) {
				cursor_ = lookahead;
				cursor_.advance();
				matched = true;
			} else if (!is_whitespace(c)) {
				break;
			}

			lookahead.advance();
		}
		return matched;
	}

	TokenKind match_dot() {
		auto backup = cursor_;
		if (cursor_.match('.')) {
			if (cursor_.match('.')) {
				return TokenKind::Ellipsis;
			}
			cursor_ = backup; // reset to ensure the extra dot is not skipped
		} else if (is_dec_digit(cursor_.peek().value_or('\0'))) {
			eat_number_literal(&is_dec_digit);
			return TokenKind::FloatLiteral;
		}

		return TokenKind::Dot;
	}

	TokenKind match_colon() {
		if (cursor_.match(':')) {
			return TokenKind::ColonColon;
		}
		return TokenKind::Colon;
	}

	TokenKind match_left_angle() {
		if (cursor_.match('<')) {
			if (cursor_.match('=')) {
				return TokenKind::LeftAngleAngleEquals;
			}
			return TokenKind::LeftAngleAngle;
		}

		if (cursor_.match('=')) {
			return TokenKind::LeftAngleEquals;
		}

		return TokenKind::LeftAngle;
	}

	TokenKind match_right_angle() {
		auto backup = cursor_;
		if (cursor_.match('>')) {
			if (cursor_.match('=')) {
				return TokenKind::RightAngleAngleEquals;
			}
			cursor_ = backup; // reset to ensure the extra right angle is not skipped
		} else if (cursor_.match('=')) {
			return TokenKind::RightAngleEquals;
		}

		return TokenKind::RightAngle;
	}

	TokenKind match_equal() {
		if (cursor_.match('=')) {
			return TokenKind::EqualsEquals;
		}
		if (cursor_.match('>')) {
			return TokenKind::ThickArrow;
		}
		return TokenKind::Equals;
	}

	TokenKind match_plus() {
		if (cursor_.match('+')) {
			return TokenKind::PlusPlus;
		}
		if (cursor_.match('=')) {
			return TokenKind::PlusEquals;
		}
		return TokenKind::Plus;
	}

	TokenKind match_minus() {
		if (cursor_.match('>')) {
			return TokenKind::ThinArrow;
		}
		if (cursor_.match('-')) {
			return TokenKind::MinusMinus;
		}
		if (cursor_.match('=')) {
			return TokenKind::MinusEquals;
		}
		return TokenKind::Minus;
	}

	TokenKind match_star() {
		if (cursor_.match('*')) {
			if (cursor_.match('=')) {
				return TokenKind::StarStarEquals;
			}
			return TokenKind::StarStar;
		}

		if (cursor_.match('=')) {
			return TokenKind::StarEquals;
		}
		return TokenKind::Star;
	}

	TokenKind match_slash() {
		if (cursor_.match('/')) {
			eat_line_comment();
			return TokenKind::LineComment;
		}

		if (cursor_.match('*')) {
			eat_block_comment();
			return TokenKind::BlockComment;
		}

		if (cursor_.match('=')) {
			return TokenKind::SlashEquals;
		}
		return TokenKind::Slash;
	}

	void eat_line_comment() {
		while (auto next = cursor_.peek()) {
			if (*next == '\n') {
				break;
			}
			cursor_.advance();
		}
	}

	void eat_block_comment() {
		auto comment_begin = cursor_.offset();

		uint32_t unclosed_count = 1;
		while (cursor_.valid()) {
			if (cursor_.match('*')) {
				if (cursor_.match('/') && --unclosed_count == 0) {
					return;
				}
			} else if (cursor_.match('/')) {
				if (cursor_.match('*')) {
					++unclosed_count;
				}
			} else {
				cursor_.advance();
			}
		}

		if (unclosed_count != 0) {
			report(Message::UnterminatedBlockComment, comment_begin);
		}
	}

	TokenKind match_percent() {
		if (cursor_.match('=')) {
			return TokenKind::PercentEquals;
		}
		return TokenKind::Percent;
	}

	TokenKind match_bang() {
		if (cursor_.match('=')) {
			return TokenKind::BangEquals;
		}
		return TokenKind::Bang;
	}

	TokenKind match_ampersand() {
		if (cursor_.match('&')) {
			return TokenKind::AmpersandAmpersand;
		}
		if (cursor_.match('=')) {
			return TokenKind::AmpersandEquals;
		}
		return TokenKind::Ampersand;
	}

	TokenKind match_pipe() {
		if (cursor_.match('|')) {
			return TokenKind::PipePipe;
		}
		if (cursor_.match('=')) {
			return TokenKind::PipeEquals;
		}
		return TokenKind::Pipe;
	}

	TokenKind match_tilde() {
		if (cursor_.match('=')) {
			return TokenKind::TildeEquals;
		}
		return TokenKind::Tilde;
	}

	void eat_quoted_sequence(char quote) {
		bool ignore_quote = false;
		while (auto next = cursor_.peek()) {
			const char c = *next;
			if (c == '\n') {
				report(Message::MissingClosingQuote, cursor_.offset());
				break;
			}

			cursor_.advance();

			if (c == '\\') {
				ignore_quote ^= true; // bool gets flipped so we correctly handle an escaped backslash within the literal
			} else if (c == quote && !ignore_quote) {
				break;
			} else if (ignore_quote) {
				ignore_quote = false;
			}
		}
	}

	void eat_unicode_token(char next, uint32_t begin_offset) {
		if (check_multibyte_utf8_value(next, &is_utf8_xid_start, begin_offset)) {
			eat_word_token_rest();
		}
	}

	template<typename... Args>
	void report(Message message, uint32_t begin_offset, Args&&... args) const {
		auto location = source_.locate(begin_offset);
		reporter_.report(message, location, std::forward<Args>(args)...);
	}
};

TokenStream lex(const SourceLock& source, Reporter& reporter) {
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero
