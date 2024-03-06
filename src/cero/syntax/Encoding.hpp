#pragma once

#include <cstdint>

namespace cero {

inline bool is_dec_digit(char c) {
	return c >= '0' && c <= '9';
}

inline bool is_hex_digit(char c) {
	return is_dec_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

inline bool is_standard_ascii(char c) {
	return static_cast<uint8_t>(c) >> 7 == 0;
}

inline bool is_ascii_word_character(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

inline bool is_whitespace(char c) {
	return c == ' ' || (c >= '\t' && c <= '\r');
}

bool is_utf8_xid_start(uint32_t encoded);
bool is_utf8_xid_continue(uint32_t encoded);

} // namespace cero
