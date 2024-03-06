#include "Reporter.hpp"

#include "cero/util/Fail.hpp"

namespace cero {

namespace {

	void verify_message_arg_count(Message message, size_t arg_count) {
		auto format = get_message_format(message);

		size_t count = 0;
		bool open = false;
		for (char c : format) {
			if (c == '{') {
				open = true;
			} else if (c == '}' && open) {
				++count;
				open = false;
			}
		}

		if (count != arg_count) {
			fail_assert("Incorrect number of message arguments.");
		}
	}

} // namespace

void MessageArgs::store_args(fmt::format_args args) {
	store.reserve(count, 0);

	int i = 0;
	while (auto arg = args.get(i++)) {
		arg.visit([&]<typename T>(const T& value) {
			if constexpr (fmt::is_formattable<T>::value) {
				store.push_back(value);
			}
		});
	}
}

void Reporter::report(Message message, CodeLocation location, MessageArgs args) {
	verify_message_arg_count(message, args.count);

	auto message_level = get_default_message_level(message);
	if (warnings_as_errors_ && message_level == MessageLevel::Warning) {
		message_level = MessageLevel::Error;
	}

	if (message_level == MessageLevel::Error) {
		has_error_reports_ = true;
	}

	auto format = get_message_format(message);
	handle_report(message_level, location, fmt::vformat(format, args.store));
}

bool Reporter::has_errors() const {
	return has_error_reports_;
}

void Reporter::set_warnings_as_errors(bool value) {
	warnings_as_errors_ = value;
}

} // namespace cero
