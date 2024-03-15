#include "SystemError.hpp"

#include "cero/util/Fail.hpp"
#include "cero/util/WinApi.win.hpp"
#include "cero/util/WinUtil.win.hpp"

namespace cero {

std::error_code get_system_error() {
	const int error = static_cast<int>(::GetLastError());
	return std::error_code(error, std::system_category());
}

std::string get_system_error_message(std::error_code error_code) {
	wchar_t* msg_buffer = nullptr;
	DWORD len = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
								 nullptr, error_code.value(), MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
								 reinterpret_cast<LPWSTR>(&msg_buffer), 0, nullptr);
	if (len == 0) {
		fail_result("Failed to translate system error message.");
	}

	auto msg = windows::utf16_to_utf8({msg_buffer, len});
	::LocalFree(msg_buffer);

	msg.erase(msg.find_last_not_of("\r\n") + 1);
	return msg;
}

} // namespace cero
