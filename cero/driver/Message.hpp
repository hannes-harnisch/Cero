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
	EscapedNonKeyword,
	ExpectFuncStructEnum,
	ExpectParenAfterFuncName,
	ExpectParamName,
	ExpectParenAfterParams,
	ExpectBraceBeforeFuncBody,
	ExpectNameAfterDeclType,
	ExpectBraceOrLineAfterStatement,
	ExpectExpr,
	ExpectNameAfterDot,
	ExpectColonAfterCondition,
	UnnecessaryColonBeforeBlock,
	ExpectParenAfterExpr,
	ExpectParenAfterCall,
	ExpectBracketAfterIndex,
	IllegalOperatorChaining,
	IllegalOperatorMixing,
};

constexpr inline LookupTable<Message, std::string_view> MESSAGE_FORMATS = []
{
	using enum Message;

	LookupTable<Message, std::string_view> t({});
	t[SourceInputTooLarge]			   = "source input is too large, limit is {} bytes";
	t[UnexpectedCharacter]			   = "unexpected character `0x{:x}`";
	t[MissingClosingQuote]			   = "missing closing quote";
	t[UnterminatedBlockComment]		   = "unterminated block comment";
	t[EscapedNonKeyword]			   = "`{}` is not a keyword that can be escaped";
	t[ExpectFuncStructEnum]			   = "expected function, struct or enum, but found {}";
	t[ExpectParenAfterFuncName]		   = "expected `(` after function name, but found {}";
	t[ExpectParamName]				   = "expected parameter name, but found {}";
	t[ExpectParenAfterParams]		   = "expected `)` after parameters, but found {}";
	t[ExpectBraceBeforeFuncBody]	   = "expected `{{` before function body, but found {}";
	t[ExpectNameAfterDeclType]		   = "expected name after type in declaration, but found {}";
	t[ExpectBraceOrLineAfterStatement] = "expected new line or `}}` after statement, but found {}";
	t[ExpectExpr]					   = "expected expression, but found {}";
	t[ExpectNameAfterDot]			   = "expected member name after `.`, but found {}";
	t[ExpectColonAfterCondition]	   = "expected `:` after condition, but found {}";
	t[UnnecessaryColonBeforeBlock]	   = "`:` between condition and block is unnecessary";
	t[ExpectParenAfterExpr]			   = "expected `)` after expression, but found {}";
	t[ExpectParenAfterCall]			   = "expected `)` after function call, but found {}";
	t[ExpectBracketAfterIndex]		   = "expected `]` after indexing, but found {}";
	t[IllegalOperatorChaining]		   = "chaining the `{}` operator is not allowed";
	t[IllegalOperatorMixing]		   = "mixing the `{}` and `{}` operators is ambiguous, could be {} or {}, consider adding "
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
