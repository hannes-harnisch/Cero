#pragma once

#include <cstdint>
#include <string_view>

namespace cero {

enum class Severity : uint8_t {
	Error,
	Warning,
	Note,
};

enum class Message : uint8_t {
	FileNotFound,
	CouldNotOpenFile,
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

std::string_view get_message_format(Message message);
Severity		 get_message_severity(Message message);
const char*		 to_string(Severity severity);
void			 verify_message_arg_count(Message message, size_t arg_count);

} // namespace cero
