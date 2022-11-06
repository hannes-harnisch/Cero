#pragma once

#include <cstdint>

char to_upper_ascii(char c);
char to_lower_ascii(char c);
bool is_dec_digit(char c);
bool is_hex_digit(char c);
bool is_standard_ascii(char c);
bool is_ascii_word_character(char c);

bool is_utf8_xid_start(uint32_t encoded);
bool is_utf8_xid_continue(uint32_t encoded);
