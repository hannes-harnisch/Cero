#pragma once

#include "driver/Source.hpp"
#include "syntax/TokenKind.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace cero {

struct Token {
	static constexpr uint32_t KindBits = 8;
	static constexpr uint32_t OffsetBits = 24;
	static constexpr uint32_t MaxLength = (1 << OffsetBits) - 1;

	TokenKind kind : KindBits = {};
	uint32_t offset : OffsetBits = {};
	uint32_t length = {};

	std::string_view get_lexeme(const Source& source) const;
	std::string to_message_string(const Source& source) const;
	std::string to_log_string(const Source& source) const;
	SourceLocation locate_in(const Source& source) const;
};

} // namespace cero
