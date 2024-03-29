#include "Lex.hpp"

#include "cero/io/Message.hpp"
#include "cero/syntax/Encoding.hpp"
#include "cero/syntax/SourceCursor.hpp"

namespace cero {

class Lexer {
public:
	Lexer(const SourceGuard& source, Reporter& reporter) :
		source_(source),
		reporter_(reporter),
		cursor_(source),
		stream_(source) {
	}

	TokenStream lex() {
		if (source_.get_length() > MaxSourceLength) {
			const auto blank = CodeLocation::blank(source_.get_name());
			reporter_.report(Message::SourceInputTooLarge, blank, MessageArgs(MaxSourceLength));
		} else {
			lex_source();
		}

		stream_.add_header(TokenKind::EndOfFile, cursor_.offset());
		return std::move(stream_);
	}

private:
	const SourceGuard& source_;
	Reporter& reporter_;
	SourceCursor cursor_;
	TokenStream stream_;

	void lex_source() {
		while (auto position = cursor_.next_position()) {
			const auto [character, offset] = position;
			switch (character) {
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
				case 'Z': lex_word(offset); break;

				case '0': lex_zero(offset); break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9': lex_number(offset); break;

				case '.': lex_dot(offset); break;
				case ':': lex_colon(offset); break;
				case ',': stream_.add_header(TokenKind::Comma, offset); break;
				case ';': stream_.add_header(TokenKind::Semicolon, offset); break;
				case '{': stream_.add_header(TokenKind::LeftBrace, offset); break;
				case '}': stream_.add_header(TokenKind::RightBrace, offset); break;
				case '(': stream_.add_header(TokenKind::LeftParen, offset); break;
				case ')': stream_.add_header(TokenKind::RightParen, offset); break;
				case '[': stream_.add_header(TokenKind::LeftBracket, offset); break;
				case ']': stream_.add_header(TokenKind::RightBracket, offset); break;
				case '<': lex_left_angle(offset); break;
				case '>': lex_right_angle(offset); break;
				case '=': lex_equal(offset); break;
				case '+': lex_plus(offset); break;
				case '-': lex_minus(offset); break;
				case '*': lex_star(offset); break;
				case '/': lex_slash(offset); break;
				case '%': lex_percent(offset); break;
				case '!': lex_bang(offset); break;
				case '&': lex_ampersand(offset); break;
				case '|': lex_pipe(offset); break;
				case '~': lex_tilde(offset); break;
				case '^': stream_.add_header(TokenKind::Caret, offset); break;
				case '?': stream_.add_header(TokenKind::QuestionMark, offset); break;
				case '@': stream_.add_header(TokenKind::At, offset); break;
				case '$': stream_.add_header(TokenKind::Dollar, offset); break;
				case '#': stream_.add_header(TokenKind::Hash, offset); break;
				case '"': lex_quoted_sequence(TokenKind::StringLiteral, offset, '"'); break;
				case '\'': lex_quoted_sequence(TokenKind::CharLiteral, offset, '\''); break;
				default: lex_unicode_name(character, offset); break;
			}
		}
	}

	void lex_word(SourceOffset offset) {
		eat_word_token_rest();
		auto length = cursor_.offset() - offset;

		auto lexeme = source_.get_text().substr(offset, length);
		auto kind = identify_keyword(lexeme);

		stream_.add_header(kind, offset);
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

	template<bool Utf8Predicate(uint32_t encoded_value)>
	bool check_multibyte_utf8_value(char character, SourceOffset offset) {
		const auto leading_byte = static_cast<uint8_t>(character);
		const auto leading_ones = static_cast<uint8_t>(std::countl_one(leading_byte));

		uint32_t encoded_value = leading_byte;
		if (leading_ones >= 2 && leading_ones <= 4) {
			auto bytes = std::bit_cast<std::array<char, 4>>(encoded_value);
			for (uint32_t i = 1; i != leading_ones; ++i) {
				bytes[i] = cursor_.next().value_or('\0');
			}

			encoded_value = std::bit_cast<uint32_t>(bytes);
			if (Utf8Predicate(encoded_value)) {
				return true;
			}
		}

		report(Message::InvalidCharacter, offset, MessageArgs(encoded_value));
		return false;
	}

	void add_variable_length_token(TokenKind kind, SourceOffset offset) {
		stream_.add_header(kind, offset);
		stream_.add_length(cursor_.offset() - offset);
	}

	void lex_zero(SourceOffset offset) {
		auto backup = cursor_;
		switch (cursor_.next().value_or('\0')) {
			case 'x':
				eat_number_literal<is_hex_digit>();
				add_variable_length_token(TokenKind::HexIntLiteral, offset);
				return;

			case 'b':
				eat_number_literal<is_dec_digit>(); // consume any decimal digit for better errors during literal parsing later
				add_variable_length_token(TokenKind::BinIntLiteral, offset);
				return;

			case 'o':
				eat_number_literal<is_dec_digit>(); // consume any decimal digit for better errors during literal parsing later
				add_variable_length_token(TokenKind::OctIntLiteral, offset);
				return;
		}
		cursor_ = backup;
		lex_number(offset);
	}

	void lex_number(SourceOffset offset) {
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

	template<bool CharPredicate(char)>
	void eat_number_literal() {
		auto lookahead = cursor_;

		while (auto next = lookahead.peek()) {
			const char c = *next;
			if (CharPredicate(c)) {
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

	void lex_dot(SourceOffset offset) {
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

	void lex_colon(SourceOffset offset) {
		if (cursor_.match(':')) {
			stream_.add_header(TokenKind::ColonColon, offset);
		} else {
			stream_.add_header(TokenKind::Colon, offset);
		}
	}

	void lex_left_angle(SourceOffset offset) {
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

	void lex_right_angle(SourceOffset offset) {
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

	void lex_equal(SourceOffset offset) {
		TokenKind kind;

		if (cursor_.match('=')) {
			kind = TokenKind::EqualsEquals;
		} else if (cursor_.match('>')) {
			kind = TokenKind::ThickArrow;
		} else {
			kind = TokenKind::Equals;
		}

		stream_.add_header(kind, offset);
	}

	void lex_plus(SourceOffset offset) {
		TokenKind kind;

		if (cursor_.match('+')) {
			kind = TokenKind::PlusPlus;
		} else if (cursor_.match('=')) {
			kind = TokenKind::PlusEquals;
		} else {
			kind = TokenKind::Plus;
		}

		stream_.add_header(kind, offset);
	}

	void lex_minus(SourceOffset offset) {
		TokenKind kind;

		if (cursor_.match('>')) {
			kind = TokenKind::ThinArrow;
		} else if (cursor_.match('-')) {
			kind = TokenKind::MinusMinus;
		} else if (cursor_.match('=')) {
			kind = TokenKind::MinusEquals;
		} else {
			kind = TokenKind::Minus;
		}

		stream_.add_header(kind, offset);
	}

	void lex_star(SourceOffset offset) {
		TokenKind kind;

		if (cursor_.match('*')) {
			if (cursor_.match('=')) {
				kind = TokenKind::StarStarEquals;
			} else {
				kind = TokenKind::StarStar;
			}
		} else if (cursor_.match('=')) {
			kind = TokenKind::StarEquals;
		} else {
			kind = TokenKind::Star;
		}

		stream_.add_header(kind, offset);
	}

	void lex_slash(SourceOffset offset) {
		if (cursor_.match('/')) {
			eat_line_comment();
			add_variable_length_token(TokenKind::LineComment, offset);
		} else if (cursor_.match('*')) {
			eat_block_comment(offset);
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

	void eat_block_comment(SourceOffset offset) {
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

		if (unclosed_count > 0) {
			report(Message::UnterminatedBlockComment, offset, {});
		}
	}

	void lex_percent(SourceOffset offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::PercentEquals, offset);
		} else {
			stream_.add_header(TokenKind::Percent, offset);
		}
	}

	void lex_bang(SourceOffset offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::BangEquals, offset);
		} else {
			stream_.add_header(TokenKind::Bang, offset);
		}
	}

	void lex_ampersand(SourceOffset offset) {
		if (cursor_.match('&')) {
			if (cursor_.match('=')) {
				stream_.add_header(TokenKind::AmpersandAmpersandEquals, offset);
			} else {
				stream_.add_header(TokenKind::AmpersandAmpersand, offset);
			}
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::AmpersandEquals, offset);
		} else {
			stream_.add_header(TokenKind::Ampersand, offset);
		}
	}

	void lex_pipe(SourceOffset offset) {
		if (cursor_.match('|')) {
			if (cursor_.match('=')) {
				stream_.add_header(TokenKind::PipePipeEquals, offset);
			} else {
				stream_.add_header(TokenKind::PipePipe, offset);
			}
		} else if (cursor_.match('=')) {
			stream_.add_header(TokenKind::PipeEquals, offset);
		} else {
			stream_.add_header(TokenKind::Pipe, offset);
		}
	}

	void lex_tilde(SourceOffset offset) {
		if (cursor_.match('=')) {
			stream_.add_header(TokenKind::TildeEquals, offset);
		} else {
			stream_.add_header(TokenKind::Tilde, offset);
		}
	}

	void lex_quoted_sequence(TokenKind kind, SourceOffset offset, char quote) {
		bool ignore_quote = false;
		while (auto next = cursor_.peek()) {
			const char c = *next;
			if (c == '\n') {
				report(Message::MissingClosingQuote, cursor_.offset(), {});
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

	void lex_unicode_name(char character, SourceOffset offset) {
		if (check_multibyte_utf8_value<is_utf8_xid_start>(character, offset)) {
			eat_word_token_rest();
		}
		add_variable_length_token(TokenKind::Name, offset);
	}

	void report(Message message, SourceOffset offset, MessageArgs args) const {
		auto location = source_.locate(offset);
		reporter_.report(message, location, std::move(args));
	}

	static TokenKind identify_keyword(std::string_view lexeme) {
		using enum TokenKind;
		switch (lexeme.length()) { // clang-format off
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
		} // clang-format on
		return Name;
	}
};

TokenStream lex(const SourceGuard& source, Reporter& reporter) {
	Lexer lexer(source, reporter);
	return lexer.lex();
}

} // namespace cero
