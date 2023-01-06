#pragma once

#include "driver/Source.hpp"
#include "syntax/Token.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace cero
{

struct LexicalToken
{
	static constexpr size_t KIND_BITS	= 8;
	static constexpr size_t LENGTH_BITS = 24;
	static constexpr size_t MAX_LENGTH	= (1 << LENGTH_BITS) - 1;

	Token	 kind : KIND_BITS	  = {};
	uint32_t length : LENGTH_BITS = 0;
	uint32_t offset				  = 0;

	std::string_view get_lexeme(const Source& source) const;
	std::string		 to_message_string(const Source& source) const;
	std::string		 to_log_string(const Source& source) const;
	SourceLocation	 locate_in(const Source& source) const;
};

} // namespace cero
