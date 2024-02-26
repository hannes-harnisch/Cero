#include "Lex.hpp"

#include "cero/io/Message.hpp"
#include "cero/syntax/Encoding.hpp"
#include "cero/syntax/SourceCursor.hpp"

namespace cero {

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

		stream_.add_header(TokenKind::EndOfFile, cursor_.offset());
		return std::move(stream_);
	}

private:
	const SourceLock& source_;
	Reporter& reporter_;
	SourceCursor cursor_;
	TokenStream stream_;

	void lex_source() {
		while (auto position = cursor_.next_position()) {
			switch (position.character) {
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
				case 'Z': lex_word(position.offset); break;

				case '0': lex_zero(position.offset); break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': lex_number(position.offset); break;

				case '.': lex_dot(position.offset); break;
				case ':': lex_colon(position.offset); break;
				case ',': stream_.add_header(TokenKind::Comma, position.offset); break;
				case ';': stream_.add_header(TokenKind::Semicolon, position.offset); break;
				case '{': stream_.add_header(TokenKind::LeftBrace, position.offset); break;
				case '}': stream_.add_header(TokenKind::RightBrace, position.offset); break;
				case '(': stream_.add_header(TokenKind::LeftParen, position.offset); break;
				case ')': stream_.add_header(TokenKind::RightParen, position.offset); break;
				case '[': stream_.add_header(TokenKind::LeftBracket, position.offset); break;
				case ']': stream_.add_header(TokenKind::RightBracket, position.offset); break;
				case '<': lex_left_angle(position.offset); break;
				case '>': lex_right_angle(position.offset); break;
				case '=': lex_equal(position.offset); break;
				case '+': lex_plus(position.offset); break;
				case '-': lex_minus(position.offset); break;
				case '*': lex_star(position.offset); break;
				case '/': lex_slash(position.offset); break;
				case '%': lex_percent(position.offset); break;
				case '!': lex_bang(position.offset); break;
				case '&': lex_ampersand(position.offset); break;
				case '|': lex_pipe(position.offset); break;
				case '~': lex_tilde(position.offset); break;
				case '^': stream_.add_header(TokenKind::Caret, position.offset); break;
				case '?': stream_.add_header(TokenKind::QuestionMark, position.offset); break;
				case '@': stream_.add_header(TokenKind::At, position.offset); break;
				case '$': stream_.add_header(TokenKind::Dollar, position.offset); break;
				case '#': stream_.add_header(TokenKind::Hash, position.offset); break;
				case '"': lex_quoted_sequence(TokenKind::StringLiteral, position.offset, '"'); break;
				case '\'': lex_quoted_sequence(TokenKind::CharLiteral, position.offset, '\''); break;
				default: lex_unicode_name(position.character, position.offset); break;
			}
		}
	}

	void lex_word(uint32_t begin_offset) {
		eat_word_token_rest();
		auto length = cursor_.offset() - begin_offset;

		auto lexeme = source_.get_text().substr(begin_offset, length);
		auto kind = identify_keyword(lexeme);

		stream_.add_header(kind, begin_offset);
		if (kind == TokenKind::Name) {
			stream_.add_length(length);
		}
	}

	void eat_word_token_rest() {
		while (auto next = cursor_.peek()) {
			const char c = *next;
			if (is_standard_ascii(c)) {
				if (!is_ascii_word_character(c)) {
					break;
				}
			} else {
				if (!check_multibyte_utf8_value<is_utf8_xid_continue>(c, cursor_.offset())) {
					break;
				}
			}
			cursor_.advance();
		}
	}

	template<bool (*UTF8_PREDICATE)(uint32_t encoded)>
	bool check_multibyte_utf8_value(char next, uint32_t begin_offset) {
		const auto leading_byte = static_cast<uint8_t>(next);
		const auto leading_ones = static_cast<uint8_t>(std::countl_one(leading_byte));

		uint32_t encoding = leading_byte;

		bool valid = false;
		if (leading_ones >= 2 && leading_ones <= 4) {
			char* bytes = reinterpret_cast<char*>(&encoding) + 1;

			for (uint32_t i = 1; i != leading_ones; ++i) {
				*bytes++ = cursor_.next().value_or('\0');
			}

			valid = UTF8_PREDICATE(encoding);
		}

		if (!valid) {
			report(Message::UnexpectedCharacter, begin_offset, encoding);
		}
		return valid;
	}

	void add_variable_length_token(TokenKind kind, uint32_t begin_offset) {
		stream_.add_header(kind, begin_offset);
		stream_.add_length(cursor_.offset() - begin_offset);
	}

	void lex_zero(uint32_t offset) {
		auto backup = cursor_;
		switch (cursor_.next().value_or('\0')) {
			case 'x':
				eat_number_literal<is_hex_digit>();
				add_variable_length_token(TokenKind::HexIntLiteral, offset);
				return;

			case 'b':
				eat_number_literal<is_dec_digit>(); // consume all decimal digits for better errors during literal parsing later
				add_variable_length_token(TokenKind::BinIntLiteral, offset);
				return;

			case 'o':
				eat_number_literal<is_dec_digit>(); // consume all decimal digits for better errors during literal parsing later
				add_variable_length_token(TokenKind::OctIntLiteral, offset);
				return;
		}
		cursor_ = backup;
		lex_number(offset);
	}

	void lex_number(uint32_t offset) {
		eat_number_literal<is_dec_digit>();
		auto cursor_at_token_end = cursor_;

		while (auto c = cursor_.peek()) {
			if (!is_whitespace(*c)) {
				break;
			}
			cursor_.advance();
		}

		auto cursor_at_dot = cursor_;

		// check for rational part
		char next = cursor_.next().value_or('\0');
		if (next == '.') {
			const bool matched_number = eat_decimal_number();
			if (matched_number) {
				add_variable_length_token(TokenKind::FloatLiteral, offset);
				return;
			} else {
				cursor_ = cursor_at_dot; // reset to dot if there's no fractional part
			}
		} else {
			cursor_ = cursor_at_token_end;
		}

		add_variable_length_token(TokenKind::DecIntLiteral, offset);
	}

	template<bool (*CHAR_PREDICATE)(char)>
	void eat_number_literal() {
		auto lookahead = cursor_;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (CHAR_PREDICATE(c)) {
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

	void lex_dot(uint32_t offset) {
		if (cursor_.match('.')) {
			if (cursor_.match('.')) {
				stream_.add_header(TokenKind::Ellipsis, offset);
			} else {
				stream_.add_header(TokenKind::Dot, offset);
				stream_.add_header(TokenKind::Dot, offset + 1);
			}
		} else if (is_dec_digit(cursor_.peek().value_or('\0'))) {
			eat_number_literal<is_dec_digit>();
			add_variable_length_token(TokenKind::FloatLiteral, offset);
		} else {
			stream_.add_header(TokenKind::Dot, offset);
		}
	}

	void lex_colon(uint32_t offset) {
		if (cursor_.match(':')) {
			stream_.add_header(TokenKind::ColonColon, offset);
		} else {
			stream_.add_header(TokenKind::Colon, offset);
		}
	}

	void lex_left_angle(uint32_t offset) {
		if (cursor_.match('<')) {
			if (cursor_.match('=')) {
				stream_.add_header(TokenKind::LeftAngleAngleEquals, offset);
			} else {
				stream_.add_header(TokenKind::LeftAngleAngle, offset);
			}
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::LeftAngleEquals, offset);
		} else {
			stream_.add_header(TokenKind::LeftAngle, offset);
		}
	}

	void lex_right_angle(uint32_t offset) {
		if (cursor_.match('>')) {
			if (cursor_.match('=')) {
				stream_.add_header(TokenKind::RightAngleAngleEquals, offset);
			} else {
				stream_.add_header(TokenKind::RightAngle, offset);
				stream_.add_header(TokenKind::RightAngle, offset + 1);
			}
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::RightAngleEquals, offset);
		} else {
			stream_.add_header(TokenKind::RightAngle, offset);
		}
	}

	void lex_equal(uint32_t offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::EqualsEquals, offset);
		} else if (cursor_.match('>')) {
			stream_.add_header(TokenKind::ThickArrow, offset);
		} else {
			stream_.add_header(TokenKind::Equals, offset);
		}
	}

	void lex_plus(uint32_t offset) {
		if (cursor_.match('+')) {
			stream_.add_header(TokenKind::PlusPlus, offset);
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::PlusEquals, offset);
		} else {
			stream_.add_header(TokenKind::Plus, offset);
		}
	}

	void lex_minus(uint32_t offset) {
		if (cursor_.match('>')) {
			stream_.add_header(TokenKind::ThinArrow, offset);
		} else if (cursor_.match('-')) {
			stream_.add_header(TokenKind::MinusMinus, offset);
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::MinusEquals, offset);
		} else {
			stream_.add_header(TokenKind::Minus, offset);
		}
	}

	void lex_star(uint32_t offset) {
		if (cursor_.match('*')) {
			if (cursor_.match('=')) {
				stream_.add_header(TokenKind::StarStarEquals, offset);
			} else {
				stream_.add_header(TokenKind::StarStar, offset);
			}
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::StarEquals, offset);
		} else {
			stream_.add_header(TokenKind::Star, offset);
		}
	}

	void lex_slash(uint32_t offset) {
		if (cursor_.match('/')) {
			eat_line_comment();
			add_variable_length_token(TokenKind::LineComment, offset);
		} else if (cursor_.match('*')) {
			eat_block_comment();
			add_variable_length_token(TokenKind::BlockComment, offset);
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::SlashEquals, offset);
		} else {
			stream_.add_header(TokenKind::Slash, offset);
		}
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

	void lex_percent(uint32_t offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::PercentEquals, offset);
		} else {
			stream_.add_header(TokenKind::Percent, offset);
		}
	}

	void lex_bang(uint32_t offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::BangEquals, offset);
		} else {
			stream_.add_header(TokenKind::Bang, offset);
		}
	}

	void lex_ampersand(uint32_t offset) {
		if (cursor_.match('&')) {
			stream_.add_header(TokenKind::AmpersandAmpersand, offset);
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::AmpersandEquals, offset);
		} else {
			stream_.add_header(TokenKind::Ampersand, offset);
		}
	}

	void lex_pipe(uint32_t offset) {
		if (cursor_.match('|')) {
			stream_.add_header(TokenKind::PipePipe, offset);
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::PipeEquals, offset);
		} else {
			stream_.add_header(TokenKind::Pipe, offset);
		}
	}

	void lex_tilde(uint32_t offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::TildeEquals, offset);
		} else {
			stream_.add_header(TokenKind::Tilde, offset);
		}
	}

	void lex_quoted_sequence(TokenKind kind, uint32_t offset, char quote) {
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

		add_variable_length_token(kind, offset);
	}

	void lex_unicode_name(char next, uint32_t begin_offset) {
		if (check_multibyte_utf8_value<is_utf8_xid_start>(next, begin_offset)) {
			eat_word_token_rest();
		}
		add_variable_length_token(TokenKind::Name, begin_offset);
	}

	template<typename... Args>
	void report(Message message, uint32_t begin_offset, Args&&... args) const {
		auto location = source_.locate(begin_offset);
		reporter_.report(message, location, std::forward<Args>(args)...);
	}

	static TokenKind identify_keyword(std::string_view lexeme) {
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
};

TokenStream lex(const SourceLock& source, Reporter& reporter) {
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero
