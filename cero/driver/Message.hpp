#pragma once

#include "util/Fail.hpp"

#include <string_view>

enum class Message
{
	IllegalChar,
	TokenTooLong,
	MissingClosingQuote,
	EscapedNonKeyword
};

constexpr std::string_view get_format(Message message)
{
	using enum Message;
	switch (message)
	{
		case IllegalChar: return "illegal character `{}`";
		case MissingClosingQuote: return "missing closing quote";
		case TokenTooLong: return "token is too long";
		case EscapedNonKeyword: return "`{}` is not a keyword that can be escaped";
	}
	fail_unreachable();
}
