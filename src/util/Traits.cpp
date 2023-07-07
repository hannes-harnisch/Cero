#include "Traits.hpp"

namespace cero {

std::string_view normalize_function_name(std::source_location location) {
	std::string_view path = location.function_name();

	constexpr std::string_view expected_call_conv = "__cdecl ";

	size_t call_conv_end = path.find(expected_call_conv);
	if (call_conv_end == std::string_view::npos)
		return path;

	size_t param_begin = path.find_last_of('(');
	size_t name_offset = call_conv_end + expected_call_conv.length();
	return path.substr(name_offset, param_begin - name_offset);
}

} // namespace cero
