#pragma once

#include "util/Fail.hpp"

#include <string_view>

enum class Message
{
	SourceInputTooLarge,
	UnexpectedCharacter,
	MissingClosingQuote,
	UnterminatedBlockComment,
	EscapedNonKeyword
};

constexpr std::string_view get_format(Message message)
{
	switch (message)
	{
		using enum Message;
		case SourceInputTooLarge: return "source input is too large, limit is {} bytes";
		case UnexpectedCharacter: return "unexpected character `0x{:x}`";
		case MissingClosingQuote: return "missing closing quote";
		case UnterminatedBlockComment: return "unterminated block comment";
		case EscapedNonKeyword: return "`{}` is not a keyword that can be escaped";
	}
	fail_unreachable();
}
