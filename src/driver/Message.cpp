#include "Message.hpp"

#include "util/Fail.hpp"

namespace cero {

std::string_view get_message_format(Message message) {
	using enum Message;
	switch (message) {
		case FileNotFound: return "file not found";
		case CouldNotOpenFile: return "could not open file (system error {})";
		case SourceInputTooLarge: return "source input is too large, limit is {} bytes";
		case UnexpectedCharacter: return "unexpected character `0x{:x}`";
		case MissingClosingQuote: return "missing closing quote";
		case UnterminatedBlockComment: return "block comment must be closed with `*/`";
		case ExpectFuncStructEnum: return "expected function, struct or enum, but found {}";
		case ExpectParenAfterFuncName: return "expected `(` after function name, but found {}";
		case ExpectType: return "expected a type, but found {}";
		case ExpectParamName: return "expected a parameter name, but found {}";
		case ExpectParenAfterParams: return "expected `)` after parameters, but found {}";
		case ExpectParenAfterOutputs: return "expected `)` after function outputs, but found {}";
		case ExpectBraceBeforeFuncBody: return "expected `{{` before function body, but found {}";
		case ExpectNameAfterLet: return "expected a name after `let` specifier, but found {}";
		case ExpectNameAfterDeclType: return "expected a name after type in declaration, but found {}";
		case ExpectExpr: return "expected an expression, but found {}";
		case ExpectSemicolon: return "expected a `;`, but found {}";
		case ExpectNameAfterDot: return "expected a member name after `.`, but found {}";
		case ExpectColonOrBlock: return "expected `:` or `{{` before control flow statement, but found {}";
		case UnnecessaryColonBeforeBlock: return "`:` is unnecessary before a block";
		case ExpectClosingParen: return "expected closing `)`, but found {}";
		case ExpectBracketAfterIndex: return "expected `]` after index expression, but found {}";
		case ExpectBracketAfterArrayBound: return "expected `]` after array bound, but found {}";
		case ExpectBraceAfterVariability: return "expected `}}` after variability arguments, but found {}";
		case ExpectArrowAfterFuncTypeParams: return "expected `->` after parameters for function type, but found {}";
		case FuncTypeDefaultArgument: return "parameter in function type cannot have default argument";
		case AmbiguousOperatorChaining: return "chaining the `{}` operator is ambiguous";
		case AmbiguousOperatorMixing: return "mixing the `{}` and `{}` operators is ambiguous";
	}
	fail_unreachable();
}

Severity get_message_severity(Message message) {
	using enum Message;
	switch (message) {
		case UnnecessaryColonBeforeBlock: return Severity::Warning;
		default: return Severity::Error;
	}
}

const char* to_string(Severity severity) {
	switch (severity) {
		case Severity::Error: return "error";
		case Severity::Warning: return "warning";
		case Severity::Note: return "note";
	}
	fail_unreachable();
}

void verify_message_arg_count(Message message, size_t arg_count) {
	size_t count = 0;

	bool open = false;
	for (char c : get_message_format(message)) {
		if (c == '{')
			open = true;
		else if (c == '}' && open) {
			++count;
			open = false;
		}
	}

	if (count != arg_count)
		fail_assert("Unexpected number of message arguments.");
}

} // namespace cero