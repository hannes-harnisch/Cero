#pragma once

#include "util/Fail.hpp"

#include <string_view>

enum class Message
{
	SourceInputTooLarge,
	IllegalChar,
	MissingClosingQuote,
	EscapedNonKeyword
};

constexpr std::string_view get_format(Message message)
{
	using enum Message;
	switch (message)
	{
		case SourceInputTooLarge: return "source input is too large";
		case IllegalChar: return "illegal character `{}`";
		case MissingClosingQuote: return "missing closing quote";
		case EscapedNonKeyword: return "`{}` is not a keyword that can be escaped";
	}
	fail_unreachable();
}
