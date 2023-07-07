#include "SystemUtil.win.hpp"

#include "util/System.win.hpp"

namespace cero::windows {

std::wstring widen_string(std::string_view str) {
	int str_length	= static_cast<int>(str.length());
	int wide_length = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), str_length, nullptr, 0);

	std::wstring wide(wide_length, L'\0');
	::MultiByteToWideChar(CP_UTF8, 0, str.data(), str_length, wide.data(), wide_length);
	return wide;
}

} // namespace cero::windows