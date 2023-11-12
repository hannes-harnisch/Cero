#include "WinUtil.win.hpp"

#include "cero/util/WinApi.win.hpp"

namespace cero::windows {

std::wstring utf8_to_utf16(std::string_view input) {
	const size_t in_len = input.length();
	std::wstring output(in_len, L'\0');

	// MultiByteToWideChar does not accept length zero
	if (in_len != 0) {
		const int len = static_cast<int>(in_len);
		const int out_len = ::MultiByteToWideChar(CP_UTF8, 0, input.data(), len, output.data(), len);

		// cut down to actual number of code units
		output.resize(static_cast<size_t>(out_len));
	}
	return output;
}

std::string utf16_to_utf8(std::wstring_view input) {
	const size_t in_len = input.length();

	// maximum number of UTF-8 code units that a UTF-16 sequence could convert to
	const size_t cap = 3 * in_len;
	std::string output(cap, '\0');

	// WideCharToMultiByte does not accept length zero
	if (in_len != 0) {
		const int out_len = ::WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(in_len), output.data(),
												  static_cast<int>(cap), nullptr, nullptr);

		// cut down to actual number of code units
		output.resize(static_cast<size_t>(out_len));
	}
	return output;
}

} // namespace cero::windows
