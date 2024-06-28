#include "Message.hpp"

#include "cero/util/Fail.hpp"

namespace cero {

std::string_view message_level_to_string(MessageLevel message_level) {
	switch (message_level) {
		case MessageLevel::Error:	return "error";
		case MessageLevel::Warning: return "warning";
		case MessageLevel::Help:	return "help";
		case MessageLevel::Note:	return "note";
	}
	fail_unreachable();
}

std::string_view get_message_format(Message message) {
	switch (message) {
		using enum Message;
		case FileNotFound:					 return "file not found";
		case CouldNotOpenFile:				 return "could not open file, system error: \"{}\"";
		case SourceInputTooLarge:			 return "source input is too large, maximum allowed is {} bytes";
		case InvalidCharacter:				 return "invalid character `0x{:x}`";
		case MissingClosingQuote:			 return "missing closing quote";
		case UnterminatedBlockComment:		 return "block comment must be closed with `*/`";
		case ExpectFuncStructEnum:			 return "expected function, struct or enum, but found {}";
		case ExpectParenAfterFuncName:		 return "expected `(` after function name, but found {}";
		case ExpectType:					 return "expected a type, but found {}";
		case ExpectParamName:				 return "expected name for parameter, but found {}";
		case ExpectParenAfterParams:		 return "expected `)` after parameters, but found {}";
		case ExpectParenAfterOutputs:		 return "expected `)` after function outputs, but found {}";
		case ExpectBraceBeforeFuncBody:		 return "expected `{{` before function body, but found {}";
		case ExpectNameAfterLet:			 return "expected a name after `let` specifier, but found {}";
		case ExpectNameAfterDeclarationType: return "expected a name after type in declaration, but found {}";
		case ExpectExpr:					 return "expected expression, but found {}";
		case ExpectSemicolon:				 return "expected a `;`, but found {}";
		case UnnecessarySemicolon:			 return "unnecessary semicolon";
		case NameCannotAppearHere:			 return "name cannot appear here";
		case ExpectNameAfterDot:			 return "expected a member name after `.`, but found {}";
		case ExpectColonOrBlockInIfExpr:	 return "expected `:`or block after `if` condition, but found {}";
		case ExpectBlockAfterIfCond:		 return "expected block after `if` condition, but found {}";
		case ExpectBlockAfterElse:			 return "expected block after `else`, but found {}";
		case UnnecessaryColonBeforeBlock:	 return "`:` is unnecessary before a block";
		case ExpectElse:					 return "expected `else` after `if` expression, but found {}";
		case ExpectBlockAfterWhileCond:		 return "expected block after `while` condition, but found {}";
		case ExpectClosingParen:			 return "expected closing `)`, but found {}";
		case ExpectBracketAfterIndex:		 return "expected `]` after index expression, but found {}";
		case ExpectBracketAfterArrayBound:	 return "expected `]` after array bound, but found {}";
		case ExpectBraceAfterPermission:	 return "expected `}}` after permission arguments, but found {}";
		case ExpectArrowAfterFuncTypeParams: return "expected `->` after parameters for function type, but found {}";
		case FuncTypeDefaultArgument:		 return "parameter in function type cannot have default argument";
		case AmbiguousOperatorMixing:		 return "mixing operator `{}` with operator `{}` is ambiguous";
		case ExpectNameForStruct:			 return "expected name for struct, but found {}";
		case ExpectNameForEnum:				 return "expected name for enum, but found {}";
	}
	fail_unreachable();
}

MessageLevel get_default_message_level(Message message) {
	switch (message) {
		using enum Message;
		case UnnecessaryColonBeforeBlock:
		case UnnecessarySemicolon:		  return MessageLevel::Warning;
		default:						  return MessageLevel::Error;
	}
}

} // namespace cero
