#pragma once

#include "cero/util/LookupTable.hpp"

#include <cstdint>
#include <string_view>
#include <type_traits>

namespace cero
{

enum class Severity
{
	Error,
	Warning,
	Note,
};

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
	ExpectParenAfterOutputs,
	ExpectBraceBeforeFuncBody,
	ExpectNameAfterLet,
	ExpectNameAfterDeclType,
	ExpectExpr,
	ExpectSemicolon,
	ExpectNameAfterDot,
	ExpectColonOrBlock,
	UnnecessaryColonBeforeBlock,
	ExpectClosingParen,
	ExpectBracketAfterIndex,
	ExpectBracketAfterArrayBound,
	ExpectBraceAfterVariability,
	ExpectArrowAfterFuncTypeParams,
	FuncTypeDefaultArgument,
	AmbiguousOperatorChaining,
	AmbiguousOperatorMixing,
};

constexpr inline LookupTable<Message, std::string_view> MESSAGE_FORMATS = []
{
	using enum Message;

	LookupTable<Message, std::string_view> t;
	t[SourceInputTooLarge]			  = "source input is too large, limit is {} bytes";
	t[UnexpectedCharacter]			  = "unexpected character `0x{:x}`";
	t[MissingClosingQuote]			  = "missing closing quote";
	t[UnterminatedBlockComment]		  = "block comment must be closed with `*/`";
	t[ExpectFuncStructEnum]			  = "expected function, struct or enum, but found {}";
	t[ExpectParenAfterFuncName]		  = "expected `(` after function name, but found {}";
	t[ExpectType]					  = "expected a type, but found {}";
	t[ExpectParamName]				  = "expected a parameter name, but found {}";
	t[ExpectParenAfterParams]		  = "expected `)` after parameters, but found {}";
	t[ExpectParenAfterOutputs]		  = "expected `)` after function outputs, but found {}";
	t[ExpectBraceBeforeFuncBody]	  = "expected `{{` before function body, but found {}";
	t[ExpectNameAfterLet]			  = "expected a name after `let` specifier, but found {}";
	t[ExpectNameAfterDeclType]		  = "expected a name after type in declaration, but found {}";
	t[ExpectExpr]					  = "expected an expression, but found {}";
	t[ExpectSemicolon]				  = "expected a `;`, but found {}";
	t[ExpectNameAfterDot]			  = "expected a member name after `.`, but found {}";
	t[ExpectColonOrBlock]			  = "expected `:` or `{{` before control flow statement, but found {}";
	t[UnnecessaryColonBeforeBlock]	  = "`:` is unnecessary before a block";
	t[ExpectClosingParen]			  = "expected closing `)`, but found {}";
	t[ExpectBracketAfterIndex]		  = "expected `]` after index expression, but found {}";
	t[ExpectBracketAfterArrayBound]	  = "expected `]` after array bound, but found {}";
	t[ExpectBraceAfterVariability]	  = "expected `}}` after variability arguments, but found {}";
	t[ExpectArrowAfterFuncTypeParams] = "expected `->` after parameters for function type, but found {}";
	t[FuncTypeDefaultArgument]		  = "parameter in function type cannot have default argument";
	t[AmbiguousOperatorChaining]	  = "chaining the `{}` operator is ambiguous";
	t[AmbiguousOperatorMixing]		  = "mixing the `{}` and `{}` operators is ambiguous";
	return t;
}();

namespace internal
{
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
} // namespace internal

template<typename... Args>
using CheckedMessage = internal::MessageChecker<std::type_identity_t<Args>...>;

} // namespace cero
