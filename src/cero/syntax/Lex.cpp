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

	TokenStream lex() && {
		if (source_.get_length() > MaxSourceLength) {
			const auto blank = CodeLocation::blank(source_.get_name());
			reporter_.report(Message::SourceInputTooLarge, blank, MessageArgs(MaxSourceLength));
		} else {
			lex_source();
		}

		stream_.add_token(TokenKind::EndOfFile, cursor_.offset());
		return std::move(stream_);
	}

private:
	const SourceGuard& source_;
	Reporter& reporter_;
	SourceCursor cursor_;
	TokenStream stream_;

	void lex_source() {
		while (auto next = cursor_.peek()) {
			const char character = *next;
			if (is_whitespace(character)) {
				cursor_.advance();
				continue;
			}

			const auto offset = cursor_.offset();
			cursor_.advance();
			switch (character) {
					// clang-format off
				case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I':
				case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
				case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z': case '_':

				case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
				case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
				case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
					lex_word(offset); break;
					// clang-format on

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

				case '(':  stream_.add_token(TokenKind::LParen, offset); break;
				case ')':  stream_.add_token(TokenKind::RParen, offset); break;
				case '[':  stream_.add_token(TokenKind::LBracket, offset); break;
				case ']':  stream_.add_token(TokenKind::RBracket, offset); break;
				case '{':  stream_.add_token(TokenKind::LBrace, offset); break;
				case '}':  stream_.add_token(TokenKind::RBrace, offset); break;
				case ',':  stream_.add_token(TokenKind::Comma, offset); break;
				case ';':  stream_.add_token(TokenKind::Semicolon, offset); break;
				case '^':  stream_.add_token(TokenKind::Caret, offset); break;
				case '?':  stream_.add_token(TokenKind::Quest, offset); break;
				case '@':  stream_.add_token(TokenKind::At, offset); break;
				case '#':  stream_.add_token(TokenKind::Hash, offset); break;
				case '$':  stream_.add_token(TokenKind::Dollar, offset); break;
				case '+':  stream_.add_token(lex_plus(), offset); break;
				case '-':  stream_.add_token(lex_minus(), offset); break;
				case '*':  stream_.add_token(lex_star(), offset); break;
				case '/':  stream_.add_token(lex_slash(offset), offset); break;
				case '%':  stream_.add_token(lex_percent(), offset); break;
				case '&':  stream_.add_token(lex_ampersand(), offset); break;
				case '|':  stream_.add_token(lex_pipe(), offset); break;
				case '~':  stream_.add_token(lex_tilde(), offset); break;
				case '!':  stream_.add_token(lex_bang(), offset); break;
				case ':':  stream_.add_token(lex_colon(), offset); break;
				case '=':  stream_.add_token(lex_equal(), offset); break;
				case '<':  stream_.add_token(lex_left_angle(), offset); break;
				case '>':  lex_right_angle(offset); break;
				case '.':  lex_dot(offset); break;
				case '"':  lex_quoted_sequence(TokenKind::StringLiteral, offset, '"'); break;
				case '\'': lex_quoted_sequence(TokenKind::CharLiteral, offset, '\''); break;
				default:   lex_unicode_name(character, offset); break;
			}
		}
	}

	void lex_word(SourceOffset offset) {
		eat_word_token_rest();

		auto length = cursor_.offset() - offset;
		auto lexeme = source_.get_text().substr(offset, length);
		auto kind = identify_keyword(lexeme);

		stream_.add_token(kind, offset);
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

	void lex_zero(SourceOffset offset) {
		if (cursor_.match('x')) {
			eat_number_literal<is_hex_digit>();
			stream_.add_token(TokenKind::HexIntLiteral, offset);
		} else if (cursor_.match('b')) {
			eat_number_literal<is_dec_digit>(); // consume any decimal digit for better errors during literal parsing later
			stream_.add_token(TokenKind::BinIntLiteral, offset);
		} else if (cursor_.match('o')) {
			eat_number_literal<is_dec_digit>(); // consume any decimal digit for better errors during literal parsing later
			stream_.add_token(TokenKind::OctIntLiteral, offset);
		} else {
			lex_number(offset);
		}
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
				stream_.add_token(TokenKind::FloatLiteral, offset);
				return;
			} else {
				cursor_ = cursor_at_dot; // reset to dot if there's no fractional part
			}
		} else {
			cursor_ = cursor_at_token_end;
		}

		stream_.add_token(TokenKind::DecIntLiteral, offset);
	}

	template<bool CharPredicate(char)>
	void eat_number_literal() {
		while (auto next = cursor_.peek()) {
			const char c = *next;
			if (!CharPredicate(c) && !is_whitespace(c)) {
				break;
			}
			cursor_.advance();
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
				stream_.add_token(TokenKind::Ellipsis, offset);
			} else {
				stream_.add_token(TokenKind::Dot, offset);
				stream_.add_token(TokenKind::Dot, offset + 1);
			}
		} else if (is_dec_digit(cursor_.peek().value_or('\0'))) {
			eat_number_literal<is_dec_digit>();
			stream_.add_token(TokenKind::FloatLiteral, offset);
		} else {
			stream_.add_token(TokenKind::Dot, offset);
		}
	}

	TokenKind lex_colon() {
		if (cursor_.match(':')) {
			return TokenKind::ColonColon;
		} else {
			return TokenKind::Colon;
		}
	}

	TokenKind lex_left_angle() {
		if (cursor_.match('<')) {
			if (cursor_.match('=')) {
				return TokenKind::LAngleLAngleEq;
			} else {
				return TokenKind::LAngleLAngle;
			}
		} else if (cursor_.match('=')) {
			return TokenKind::LAngleEq;
		} else {
			return TokenKind::LAngle;
		}
	}

	void lex_right_angle(SourceOffset offset) {
		if (cursor_.match('>')) {
			if (cursor_.match('=')) {
				stream_.add_token(TokenKind::RAngleRAngleEq, offset);
			} else {
				stream_.add_token(TokenKind::RAngle, offset);
				stream_.add_token(TokenKind::RAngle, offset + 1);
			}
		} else if (cursor_.match('=')) {
			stream_.add_token(TokenKind::RAngleEq, offset);
		} else {
			stream_.add_token(TokenKind::RAngle, offset);
		}
	}

	TokenKind lex_equal() {
		if (cursor_.match('=')) {
			return TokenKind::EqEq;
		} else if (cursor_.match('>')) {
			return TokenKind::ThickArrow;
		} else {
			return TokenKind::Eq;
		}
	}

	TokenKind lex_plus() {
		if (cursor_.match('+')) {
			return TokenKind::PlusPlus;
		} else if (cursor_.match('=')) {
			return TokenKind::PlusEq;
		} else {
			return TokenKind::Plus;
		}
	}

	TokenKind lex_minus() {
		if (cursor_.match('>')) {
			return TokenKind::ThinArrow;
		} else if (cursor_.match('-')) {
			return TokenKind::MinusMinus;
		} else if (cursor_.match('=')) {
			return TokenKind::MinusEq;
		} else {
			return TokenKind::Minus;
		}
	}

	TokenKind lex_star() {
		if (cursor_.match('*')) {
			if (cursor_.match('=')) {
				return TokenKind::StarStarEq;
			} else {
				return TokenKind::StarStar;
			}
		} else if (cursor_.match('=')) {
			return TokenKind::StarEq;
		} else {
			return TokenKind::Star;
		}
	}

	TokenKind lex_slash(SourceOffset offset) {
		if (cursor_.match('/')) {
			eat_line_comment();
			return TokenKind::LineComment;
		} else if (cursor_.match('*')) {
			eat_block_comment(offset);
			return TokenKind::BlockComment;
		} else if (cursor_.match('=')) {
			return TokenKind::SlashEq;
		} else {
			return TokenKind::Slash;
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

		report(Message::UnterminatedBlockComment, offset, {});
	}

	TokenKind lex_percent() {
		if (cursor_.match('=')) {
			return TokenKind::PercentEq;
		} else {
			return TokenKind::Percent;
		}
	}

	TokenKind lex_bang() {
		if (cursor_.match('=')) {
			return TokenKind::BangEq;
		} else {
			return TokenKind::Bang;
		}
	}

	TokenKind lex_ampersand() {
		if (cursor_.match('&')) {
			if (cursor_.match('=')) {
				return TokenKind::AmpAmpEq;
			} else {
				return TokenKind::AmpAmp;
			}
		} else if (cursor_.match('=')) {
			return TokenKind::AmpEq;
		} else {
			return TokenKind::Amp;
		}
	}

	TokenKind lex_pipe() {
		if (cursor_.match('|')) {
			if (cursor_.match('=')) {
				return TokenKind::PipePipeEq;
			} else {
				return TokenKind::PipePipe;
			}
		} else if (cursor_.match('=')) {
			return TokenKind::PipeEq;
		} else {
			return TokenKind::Pipe;
		}
	}

	TokenKind lex_tilde() {
		if (cursor_.match('=')) {
			return TokenKind::TildeEq;
		} else {
			return TokenKind::Tilde;
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

		stream_.add_token(kind, offset);
	}

	void lex_unicode_name(char character, SourceOffset offset) {
		if (check_multibyte_utf8_value<is_utf8_xid_start>(character, offset)) {
			eat_word_token_rest();
		}
		stream_.add_token(TokenKind::Name, offset);
	}

	void report(Message message, SourceOffset offset, MessageArgs args) {
		auto location = source_.locate(offset);
		reporter_.report(message, location, std::move(args));
		stream_.has_errors_ = true;
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
				break; // clang-format on
		}
		return Name;
	}
};

TokenStream lex(const SourceGuard& source, Reporter& reporter) {
	return Lexer(source, reporter).lex();
}

} // namespace cero
