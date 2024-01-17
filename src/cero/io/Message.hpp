#pragma once

#include <cstdint>
#include <string_view>

namespace cero {

enum class Severity : uint8_t {
	Error,
	Warning,
	Note,
};

std::string_view severity_to_string(Severity severity);

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
	ExpectColonInIfExpr,
	ExpectColonOrBlock,
	UnnecessaryColonBeforeBlock,
	UnnecessarySemicolon,
	ExpectElse,
	ExpectClosingParen,
	ExpectBracketAfterIndex,
	ExpectBracketAfterArrayBound,
	ExpectBraceAfterPermission,
	ExpectArrowAfterFuncTypeParams,
	FuncTypeDefaultArgument,
	AmbiguousOperatorMixing,
	ExpectNameForStruct,
	ExpectNameForEnum,
};

std::string_view get_message_format(Message message);
Severity get_message_severity(Message message);

} // namespace cero
