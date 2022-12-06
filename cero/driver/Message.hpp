#pragma once

#include "util/Fail.hpp"
#include "util/LookupTable.hpp"

#include <string_view>

enum class Message : uint8_t
{
	SourceInputTooLarge,
	UnexpectedCharacter,
	MissingClosingQuote,
	UnterminatedBlockComment,
	ExpectFuncStructEnum,
	ExpectParenAfterFuncName,
	ExpectType,
	ExpectParamName,
	ExpectParenAfterParams,
	ExpectBraceBeforeFuncBody,
	ExpectNameAfterDeclType,
	ExpectExpr,
	ExpectNameAfterDot,
	ExpectColonAfterCondition,
	UnnecessaryColonBeforeBlock,
	ExpectParenAfterGroup,
	ExpectParenAfterCall,
	ExpectBracketAfterIndex,
	ExpectBracketAfterArrayCount,
	IllegalOperatorChaining,
	IllegalOperatorMixing,
};

constexpr inline LookupTable<Message, std::string_view> MESSAGE_FORMATS = []
{
	using enum Message;

	LookupTable<Message, std::string_view> t({});
	t[SourceInputTooLarge]			= "source input is too large, limit is {} bytes";
	t[UnexpectedCharacter]			= "unexpected character `0x{:x}`";
	t[MissingClosingQuote]			= "missing closing quote";
	t[UnterminatedBlockComment]		= "block comment must be closed with `*/`";
	t[ExpectFuncStructEnum]			= "expected function, struct or enum, but found {}";
	t[ExpectParenAfterFuncName]		= "expected `(` after function name, but found {}";
	t[ExpectType]					= "expected a type, but found {}";
	t[ExpectParamName]				= "expected a parameter name, but found {}";
	t[ExpectParenAfterParams]		= "expected `)` after parameters, but found {}";
	t[ExpectBraceBeforeFuncBody]	= "expected `{{` before function body, but found {}";
	t[ExpectNameAfterDeclType]		= "expected a name after type in declaration, but found {}";
	t[ExpectExpr]					= "expected an expression, but found {}";
	t[ExpectNameAfterDot]			= "expected a member name after `.`, but found {}";
	t[ExpectColonAfterCondition]	= "expected `:` after condition, but found {}";
	t[UnnecessaryColonBeforeBlock]	= "`:` between condition and block is unnecessary";
	t[ExpectParenAfterGroup]		= "expected `)` after grouping expression, but found {}";
	t[ExpectParenAfterCall]			= "expected `)` after call expression, but found {}";
	t[ExpectBracketAfterIndex]		= "expected `]` after index expression, but found {}";
	t[ExpectBracketAfterArrayCount] = "expected `]` after count in array type, but found {}";
	t[IllegalOperatorChaining]		= "chaining the `{}` operator is not allowed";
	t[IllegalOperatorMixing]		= "mixing the `{}` and `{}` operators is ambiguous, could be {} or {}, consider adding "
									  "parentheses";
	return t;
}();

template<typename... Args>
struct MessageChecker
{
	Message value;

	consteval MessageChecker(Message message) :
		value(message)
	{
		size_t count = 0;
		bool   open	 = false;

		for (char c : MESSAGE_FORMATS[message])
		{
			if (c == '{')
				open = true;
			else if (c == '}' && open)
			{
				++count;
				open = false;
			}
		}

		if (count != sizeof...(Args))
			number_of_provided_arguments_does_not_match_required_argument_count_for_this_message();
	}

private:
	void number_of_provided_arguments_does_not_match_required_argument_count_for_this_message()
	{}
};

template<typename... Args>
using CheckedMessage = MessageChecker<std::type_identity_t<Args>...>;
