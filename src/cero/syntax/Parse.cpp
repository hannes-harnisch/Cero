#include "Parse.hpp"

#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Literal.hpp"
#include "cero/syntax/TokenCursor.hpp"
#include "cero/util/Algorithm.hpp"
#include "cero/util/Fail.hpp"
#include "cero/util/ScopedAssign.hpp"

namespace cero {

enum class Precedence : uint8_t {
	Statement,
	Assignment,
	Logical,
	Comparison,
	AdditiveOrBitwise,
	Multiplicative,
	Prefix,
	Postfix
};

struct ParseError {};

consteval Precedence lookup_precedence_for_associativity(BinaryOperator op) {
	switch (op) {
		using enum BinaryOperator;
		case Add:
		case Sub:			 return Precedence::AdditiveOrBitwise;
		case Mul:
		case Div:
		case Rem:
		case Pow:			 return Precedence::Multiplicative; // using a lower precedence for power here makes it right-associative
		case BitAnd:
		case BitOr:
		case Xor:
		case Shl:
		case Shr:			 return Precedence::AdditiveOrBitwise;
		case Eq:
		case NotEq:
		case Less:
		case LessEq:
		case Greater:
		case GreaterEq:		 return Precedence::Comparison;
		case LogicAnd:
		case LogicOr:		 return Precedence::Logical;
		case Assign:
		case AddAssign:
		case SubAssign:
		case MulAssign:
		case DivAssign:
		case RemAssign:
		case PowAssign:
		case BitAndAssign:
		case BitOrAssign:
		case XorAssign:
		case ShlAssign:
		case ShrAssign:
		case LogicAndAssign:
		case LogicOrAssign:	 return Precedence::Assignment;
	}
}

class Parser {
public:
	Parser(const TokenStream& token_stream, const SourceGuard& source, Reporter& reporter) :
		source_(source),
		reporter_(reporter),
		cursor_(token_stream),
		ast_(token_stream) {
	}

	Ast parse() && {
		auto root_idx = ast_.store(AstRoot {});

		uint16_t num_definitions = 0;
		while (!cursor_.match(TokenKind::EndOfFile)) {
			try {
				parse_definition();
				++num_definitions;
			} catch (ParseError) {
				recover_at_definition_scope();
			}
		}

		auto& root = ast_.get(root_idx).as<AstRoot>();
		root.num_definitions = num_definitions;

		return std::move(ast_);
	}

private:
	const SourceGuard& source_;
	Reporter& reporter_;
	TokenCursor cursor_;
	Ast ast_;
	bool is_looking_ahead_ = false;
	bool is_binding_allowed_ = true;
	uint32_t open_angles_ = 0;

	void parse_definition() {
		auto offset = cursor_.peek_offset();

		auto access_specifier = AccessSpecifier::None;
		if (cursor_.match(TokenKind::Private)) {
			access_specifier = AccessSpecifier::Private;
		} else if (cursor_.match(TokenKind::Public)) {
			access_specifier = AccessSpecifier::Public;
		}

		auto name = cursor_.match_name(source_);
		if (!name.empty()) {
			parse_function(offset, access_specifier, name);
		} else if (cursor_.match(TokenKind::Struct)) {
			parse_struct(offset, access_specifier);
		} else if (cursor_.match(TokenKind::Enum)) {
			parse_enum(offset, access_specifier);
		} else {
			report_expectation(Message::ExpectFuncStructEnum);
			throw ParseError();
		}
	}

	void recover_at_definition_scope() {
		static constexpr TokenKind recovery_tokens[] {TokenKind::Public, TokenKind::Private, TokenKind::Struct, TokenKind::Enum,
													  TokenKind::EndOfFile};

		TokenKind kind;
		do {
			cursor_.advance();
			kind = cursor_.peek_kind();
		} while (!contains(recovery_tokens, kind));
	}

	void parse_struct(SourceOffset offset, AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForStruct);
		auto node_idx = ast_.store(AstStructDefinition {offset, access_specifier, name});

		auto& struct_def = ast_.get(node_idx).as<AstStructDefinition>();
		std::ignore = struct_def;
		to_do();
	}

	void parse_enum(SourceOffset offset, AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForEnum);
		auto node_idx = ast_.store(AstEnumDefinition {offset, access_specifier, name});

		auto& enum_def = ast_.get(node_idx).as<AstEnumDefinition>();
		std::ignore = enum_def;
		to_do();
	}

	void parse_function(SourceOffset offset, AccessSpecifier access_specifier, StringId name) {
		auto node_idx = ast_.store(AstFunctionDefinition {offset, access_specifier, name});

		expect(TokenKind::LParen, Message::ExpectParenAfterFuncName);

		auto num_parameters = parse_function_definition_parameters();
		auto num_outputs = parse_function_definition_outputs();
		expect(TokenKind::LBrace, Message::ExpectBraceBeforeFuncBody);

		auto num_statements = parse_block();

		auto& func_def = ast_.get(node_idx).as<AstFunctionDefinition>();
		func_def.num_parameters = num_parameters;
		func_def.num_outputs = num_outputs;
		func_def.num_statements = num_statements;
	}

	uint16_t parse_function_definition_parameters() {
		uint16_t num_parameters = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_function_definition_parameter();
				++num_parameters;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectParenAfterParams);
		}
		return num_parameters;
	}

	void parse_function_definition_parameter() {
		auto offset = cursor_.peek_offset();

		auto specifier = ParameterSpecifier::None;
		if (cursor_.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor_.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto node_idx = ast_.store(AstFunctionParameter {offset, specifier});

		parse_type();
		auto name = expect_name(Message::ExpectParamName);

		bool has_default_argument = cursor_.match(TokenKind::Eq);
		if (has_default_argument) {
			parse_subexpression();
		}

		auto& parameter = ast_.get(node_idx).as<AstFunctionParameter>();
		parameter.name = name;
		parameter.has_default_argument = has_default_argument;

		// abort parsing here and start next definition so we don't accumulate errors in a malformed signature
		if (name.empty()) {
			throw ParseError();
		}
	}

	uint16_t parse_function_definition_outputs() {
		uint16_t num_outputs = 0;
		if (cursor_.match(TokenKind::ThinArrow)) {
			do {
				parse_function_definition_output();
				++num_outputs;
			} while (cursor_.match(TokenKind::Comma));
		}
		return num_outputs;
	}

	void parse_function_definition_output() {
		auto offset = cursor_.peek_offset();
		auto node_idx = ast_.store(AstFunctionOutput {offset});

		parse_type();
		auto name = cursor_.match_name(source_);

		auto& output = ast_.get(node_idx).as<AstFunctionOutput>();
		output.name = name;
	}

	uint32_t parse_block() {
		ScopedAssign _1(open_angles_, 0);
		ScopedAssign _2(is_binding_allowed_, true);

		uint32_t num_statements = 0;
		while (!cursor_.match(TokenKind::RBrace)) {
			try {
				parse_statement();
				++num_statements;
			} catch (ParseError) {
				bool at_end = recover_at_statement_scope();
				if (at_end) {
					break;
				}
			}
		}
		return num_statements;
	}

	bool recover_at_statement_scope() {
		TokenKind kind = cursor_.peek_kind();
		while (kind != TokenKind::EndOfFile) {
			if (kind == TokenKind::Semicolon) {
				cursor_.advance();
				return false;
			}
			if (kind == TokenKind::RBrace) {
				return false;
			}

			cursor_.advance();
			kind = cursor_.peek_kind();
		}
		return true;
	}

	void parse_statement() {
		auto [parse_method, parses_full_stmt] = lookup_statement_parse_method();

		auto offset = cursor_.peek_offset();
		auto prev_expr = (this->*parse_method)();

		if (!parses_full_stmt) {
			auto name_offset = cursor_.peek_offset();
			auto name = cursor_.match_name(source_);
			if (!name.empty()) {
				on_trailing_name(offset, prev_expr, name, name_offset);
			}
			expect(TokenKind::Semicolon, Message::ExpectSemicolon);
		}
	}

	/// Parse method for grammar rules determined by the kind of their first token, like prefix operators
	using HeadParseMethod = Ast::NodeIndex (Parser::*)();

	/// Parse method for grammar rules determined by the kind of token appearing after another expression, like infix operators
	using TailParseMethod = void (Parser::*)(Ast::NodeIndex head_begin, SourceOffset offset);

	struct TailParseRule {
		Precedence precedence = {};
		TailParseMethod method = nullptr;
	};

	struct StatementParseMethod {
		HeadParseMethod parse_method = nullptr;
		bool parses_full_stmt = false;
	};

	StatementParseMethod lookup_statement_parse_method() {
		auto next = cursor_.peek_kind();
		switch (next) {
			using enum TokenKind;
			case If:	 return {&Parser::on_if_stmt, true};
			case For:	 return {&Parser::on_for, true};
			case While:	 return {&Parser::on_while, true};
			case LBrace: return {&Parser::on_left_brace, true};
			case Let:	 return {&Parser::on_let};
			case Var:	 return {&Parser::on_var};
			case Const:	 return {&Parser::on_const};
			case Static: return {&Parser::on_static};
			default:	 return {&Parser::parse_expression_or_binding};
		}
	}

	void on_trailing_name(SourceOffset offset, Ast::NodeIndex prev_expr, StringId name, SourceOffset name_offset) {
		auto kind = ast_.get(prev_expr).get_kind();

		static constexpr AstNodeKind type_expr_kinds[] {AstNodeKind::NameExpr,		  AstNodeKind::GenericNameExpr,
														AstNodeKind::MemberExpr,	  AstNodeKind::ArrayTypeExpr,
														AstNodeKind::PointerTypeExpr, AstNodeKind::FunctionTypeExpr};
		if (!contains(type_expr_kinds, kind)) {
			auto location = source_.locate(name_offset);
			report(Message::NameCannotAppearHere, location, {});
			throw ParseError();
		}

		auto node_idx = ast_.store_parent_of(prev_expr, AstBindingStatement {offset, BindingSpecifier::Let, true, name});

		bool has_initializer = cursor_.match(TokenKind::Eq);
		if (has_initializer) {
			parse_subexpression();
		}

		auto& binding_stmt = ast_.get(node_idx).as<AstBindingStatement>();
		binding_stmt.has_initializer = has_initializer;
	}

	Ast::NodeIndex parse_expression_or_binding() {
		ScopedAssign _(is_binding_allowed_, true);
		return parse_expression(Precedence::Statement);
	}

	Ast::NodeIndex parse_subexpression(Precedence precedence = Precedence::Statement) {
		ScopedAssign _(is_binding_allowed_, false);
		return parse_expression(precedence);
	}

	Ast::NodeIndex parse_expression(Precedence precedence) {
		auto next = cursor_.peek();

		auto head_parse_method = lookup_head_parse_method(next.kind);
		if (head_parse_method == nullptr) {
			report_expectation(Message::ExpectExpr);
			throw ParseError();
		}

		auto expression = (this->*head_parse_method)();
		while (auto parse_method = get_next_tail_parse_method(precedence)) {
			(this->*parse_method)(expression, next.offset);
		}

		return expression;
	}

	static HeadParseMethod lookup_head_parse_method(TokenKind kind) {
		switch (kind) {
			using enum TokenKind;
			case Name:			return &Parser::on_name;
			case If:			return &Parser::on_if_expr;
			case Var:			return &Parser::on_permission;
			case DecIntLiteral: return &Parser::on_numeric_literal<evaluate_dec_int_literal, NumericLiteralKind::Decimal>;
			case HexIntLiteral: return &Parser::on_numeric_literal<evaluate_hex_int_literal, NumericLiteralKind::Hexadecimal>;
			case BinIntLiteral: return &Parser::on_numeric_literal<evaluate_bin_int_literal, NumericLiteralKind::Binary>;
			case OctIntLiteral: return &Parser::on_numeric_literal<evaluate_oct_int_literal, NumericLiteralKind::Octal>;
			case FloatLiteral:	return &Parser::on_numeric_literal<evaluate_float_literal, NumericLiteralKind::Float>;
			case CharLiteral:	return &Parser::on_numeric_literal<evaluate_char_literal, NumericLiteralKind::Character>;
			case StringLiteral: return &Parser::on_string_literal;
			case LBrace:		return &Parser::on_left_brace;
			case LParen:		return &Parser::on_prefix_left_paren;
			case LBracket:		return &Parser::on_prefix_left_bracket;
			case Break:			return &Parser::on_break;
			case Continue:		return &Parser::on_continue;
			case Return:		return &Parser::on_return;
			case Throw:			return &Parser::on_throw;
			case Amp:			return &Parser::on_prefix_operator<UnaryOperator::Addr>;
			case Minus:			return &Parser::on_prefix_operator<UnaryOperator::Neg>;
			case Tilde:			return &Parser::on_prefix_operator<UnaryOperator::Not>;
			case PlusPlus:		return &Parser::on_prefix_operator<UnaryOperator::PreInc>;
			case MinusMinus:	return &Parser::on_prefix_operator<UnaryOperator::PreDec>;
			case Caret:			return &Parser::on_caret;
			default:			return nullptr;
		}
	}

	TailParseMethod get_next_tail_parse_method(Precedence current_precedence) {
		auto token = cursor_.peek();

		TailParseRule rule;
		switch (token.kind) {
			using enum TokenKind;
			using enum Precedence;
			default:			 rule = {Statement, nullptr}; break;
			case Eq:			 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::Assign>}; break;
			case PlusEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::AddAssign>}; break;
			case MinusEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::SubAssign>}; break;
			case StarEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::MulAssign>}; break;
			case SlashEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::DivAssign>}; break;
			case PercentEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::RemAssign>}; break;
			case StarStarEq:	 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::PowAssign>}; break;
			case AmpEq:			 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::BitAndAssign>}; break;
			case PipeEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::BitOrAssign>}; break;
			case TildeEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::XorAssign>}; break;
			case LAngleLAngleEq: rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::ShlAssign>}; break;
			case RAngleRAngleEq: rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::ShrAssign>}; break;
			case AmpAmpEq:		 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::LogicAndAssign>}; break;
			case PipePipeEq:	 rule = {Assignment, &Parser::on_binary_operator<BinaryOperator::LogicOrAssign>}; break;
			case AmpAmp:		 rule = {Logical, &Parser::on_binary_operator<BinaryOperator::LogicAnd>}; break;
			case PipePipe:		 rule = {Logical, &Parser::on_binary_operator<BinaryOperator::LogicOr>}; break;
			case EqEq:			 rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::Eq>}; break;
			case BangEq:		 rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::NotEq>}; break;
			case LAngle:		 rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::Less>}; break;
			case LAngleEq:		 rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::LessEq>}; break;
			case RAngleEq:		 rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::GreaterEq>}; break;
			case Plus:			 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Add>}; break;
			case Minus:			 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Sub>}; break;
			case Amp:			 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::BitAnd>}; break;
			case Pipe:			 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::BitOr>}; break;
			case Tilde:			 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Xor>}; break;
			case LAngleLAngle:	 rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Shl>}; break;
			case Star:			 rule = {Multiplicative, &Parser::on_binary_operator<BinaryOperator::Mul>}; break;
			case Slash:			 rule = {Multiplicative, &Parser::on_binary_operator<BinaryOperator::Div>}; break;
			case Percent:		 rule = {Multiplicative, &Parser::on_binary_operator<BinaryOperator::Rem>}; break;
			case StarStar:		 rule = {Prefix, &Parser::on_binary_operator<BinaryOperator::Pow>}; break;
			case Caret:			 rule = {Postfix, &Parser::on_postfix_operator<UnaryOperator::Deref>}; break;
			case PlusPlus:		 rule = {Postfix, &Parser::on_postfix_operator<UnaryOperator::PostInc>}; break;
			case MinusMinus:	 rule = {Postfix, &Parser::on_postfix_operator<UnaryOperator::PostDec>}; break;
			case Dot:			 rule = {Postfix, &Parser::on_dot}; break;
			case LParen:		 rule = {Postfix, &Parser::on_infix_left_paren}; break;
			case LBracket:		 rule = {Postfix, &Parser::on_infix_left_bracket}; break;
			case RAngle:
				// check for unclosed angle brackets so the last one gets closed instead of parsing a greater-than expression
				if (open_angles_ > 0) {
					return nullptr;
				} else {
					auto next = cursor_.peek_ahead();
					if (next.kind == TokenKind::RAngle && next.offset == token.offset + 1) {
						rule = {AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Shr>};
					} else {
						rule = {Comparison, &Parser::on_binary_operator<BinaryOperator::Greater>};
					}
				}
				break;
		}

		if (current_precedence < rule.precedence) {
			return rule.method;
		} else {
			return nullptr;
		}
	}

	Ast::NodeIndex on_if_stmt() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstIfExpr {token.offset});

		parse_expression_or_binding();
		expect(TokenKind::LBrace, Message::ExpectBlockAfterIfCond);
		const auto num_then_stmts = parse_block();

		uint32_t num_else_stmts = 0;
		if (cursor_.match(TokenKind::Else)) {
			expect(TokenKind::LBrace, Message::ExpectBlockAfterElse);
			num_else_stmts = parse_block();
		}

		auto& if_expr = ast_.get(node_idx).as<AstIfExpr>();
		if_expr.num_then_statements = num_then_stmts;
		if_expr.num_else_statements = num_else_stmts;

		return node_idx;
	}

	Ast::NodeIndex on_if_expr() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstIfExpr {token.offset, 1, 1});

		parse_expression_or_binding();

		if (auto colon = cursor_.match_token(TokenKind::Colon)) {
			if (cursor_.peek_kind() == TokenKind::LBrace) {
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source_), {});
			}
		} else {
			if (cursor_.peek_kind() != TokenKind::LBrace) {
				report_expectation(Message::ExpectColonOrBlockInIfExpr);
			}
		}
		parse_subexpression();

		expect(TokenKind::Else, Message::ExpectElse);
		parse_subexpression();

		return node_idx;
	}

	Ast::NodeIndex on_while() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstWhileLoop {token.offset});

		parse_expression_or_binding();
		expect(TokenKind::LBrace, Message::ExpectBlockAfterWhileCond);
		auto num_statements = parse_block();

		auto& while_loop = ast_.get(node_idx).as<AstWhileLoop>();
		while_loop.num_statements = num_statements;

		return node_idx;
	}

	Ast::NodeIndex on_for() {
		cursor_.advance();

		to_do();
	}

	Ast::NodeIndex on_left_brace() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstBlockStatement {token.offset});

		auto num_statements = parse_block();

		auto& block = ast_.get(node_idx).as<AstBlockStatement>();
		block.num_statements = num_statements;

		return node_idx;
	}

	Ast::NodeIndex on_let() {
		auto token = cursor_.next();
		auto name = expect_name(Message::ExpectNameAfterLet);
		auto node_idx = ast_.store(AstBindingStatement {token.offset, BindingSpecifier::Let, false, name});

		bool has_initializer = cursor_.match(TokenKind::Eq);
		if (has_initializer) {
			parse_subexpression();
		}

		auto& let_stmt = ast_.get(node_idx).as<AstBindingStatement>();
		let_stmt.has_initializer = has_initializer;

		return node_idx;
	}

	Ast::NodeIndex on_var() {
		auto token = cursor_.next();

		if (cursor_.peek_kind() == TokenKind::LBrace) {
			return parse_permission(token.offset);
		}

		return parse_binding(token.offset, BindingSpecifier::Var);
	}

	Ast::NodeIndex on_const() {
		auto token = cursor_.next();

		return parse_binding(token.offset, BindingSpecifier::Const);
	}

	Ast::NodeIndex on_static() {
		auto token = cursor_.next();

		auto specifier = cursor_.match(TokenKind::Var) ? BindingSpecifier::StaticVar : BindingSpecifier::Static;
		return parse_binding(token.offset, specifier);
	}

	Ast::NodeIndex parse_binding(SourceOffset offset, BindingSpecifier specifier) {
		auto node_idx = ast_.store(AstBindingStatement {offset, specifier});

		bool has_type;
		bool has_initializer;

		auto lookahead = cursor_;
		auto name = lookahead.match_name(source_);
		if (!name.empty() && lookahead.match(TokenKind::Eq)) {
			cursor_ = lookahead;

			parse_subexpression();

			has_type = false;
			has_initializer = true;
		} else {
			parse_type();
			name = expect_name(Message::ExpectNameAfterDeclarationType);

			has_type = true;
			has_initializer = cursor_.match(TokenKind::Eq);
			if (has_initializer) {
				parse_subexpression();
			}
		}

		auto& binding = ast_.get(node_idx).as<AstBindingStatement>();
		binding.has_type = has_type;
		binding.has_initializer = has_initializer;
		binding.name = name;

		return node_idx;
	}

	Ast::NodeIndex on_name() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();
		return parse_name(token.offset, lexeme);
	}

	Ast::NodeIndex parse_name(SourceOffset offset, StringId name) {
		if (cursor_.peek_kind() == TokenKind::LAngle) {
			return parse_generic_name(offset, name);
		}

		return ast_.store(AstNameExpr {offset, name});
	}

	Ast::NodeIndex parse_generic_name(SourceOffset offset, StringId name) {
		ScopedAssign _(open_angles_, open_angles_ + 1);

		auto before_lookahead = cursor_;
		cursor_.advance(); // consume left angle bracket

		bool is_generic = true;
		if (!cursor_.match(TokenKind::RAngle)) {
			const auto lookahead_idx = ast_.next_index();
			is_generic = lookahead_whether_is_generic();

			cursor_ = before_lookahead;
			ast_.undo_nodes_from_lookahead(lookahead_idx);
		}

		if (is_generic) {
			auto node_idx = ast_.store(AstGenericNameExpr {offset, name});

			cursor_.advance(); // consume left angle bracket

			uint16_t num_generic_args = 0;
			do {
				parse_subexpression();
				++num_generic_args;
			} while (cursor_.match(TokenKind::Comma));

			cursor_.advance(); // consume right angle bracket

			auto& generic_name_expr = ast_.get(node_idx).as<AstGenericNameExpr>();
			generic_name_expr.num_generic_args = num_generic_args;

			return node_idx;
		} else {
			return ast_.store(AstNameExpr {offset, name});
		}
	}

	bool lookahead_whether_is_generic() {
		ScopedAssign _(is_looking_ahead_, true);

		do {
			parse_subexpression();
		} while (cursor_.match(TokenKind::Comma));

		if (cursor_.match(TokenKind::RAngle)) {
			static constexpr TokenKind ForbiddenAfterGeneric[] {TokenKind::DecIntLiteral, TokenKind::HexIntLiteral,
																 TokenKind::BinIntLiteral, TokenKind::OctIntLiteral,
																 TokenKind::FloatLiteral,  TokenKind::CharLiteral,
																 TokenKind::StringLiteral, TokenKind::Minus,
																 TokenKind::Tilde,		   TokenKind::Amp,
																 TokenKind::PlusPlus,	   TokenKind::MinusMinus};

			auto next = cursor_.peek_kind();

			// allow statements with generics like `a<b, c> d;`, but comparison expressions like `f(a < b, c > d)`
			return (is_binding_allowed_ || next != TokenKind::Name)
				   // allow comparison expressions like `f(a < b, c > 0)`
				   && !contains(ForbiddenAfterGeneric, next)
				   // allow comparison expressions like `a < b >> c`
				   && (next != TokenKind::RAngle || open_angles_ > 1);
		} else {
			return false;
		}
	}

	template<void LiteralParseFn(std::string_view), NumericLiteralKind Kind>
	Ast::NodeIndex on_numeric_literal() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();
		LiteralParseFn(lexeme); // TODO

		return ast_.store(AstNumericLiteralExpr {token.offset, Kind});
	}

	Ast::NodeIndex on_string_literal() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();

		return ast_.store(AstStringLiteralExpr {token.offset, evaluate_string_literal(lexeme)});
	}

	Ast::NodeIndex on_prefix_left_paren() { // TODO: function type
		ScopedAssign _(open_angles_, 0);

		auto token = cursor_.next();
		auto node_idx = ast_.store(AstGroupExpr {token.offset});

		uint16_t num_args = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_subexpression();
				++num_args;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectClosingParen);
		}

		auto& group_expr = ast_.get(node_idx).as<AstGroupExpr>();
		group_expr.num_args = num_args;

		return node_idx;
	}

	Ast::NodeIndex on_prefix_left_bracket() {
		auto token = cursor_.next();
		// TODO: array literal
		return parse_array_type(token.offset);
	}

	uint16_t parse_bracketed_arguments() {
		ScopedAssign _(open_angles_, 0);

		uint16_t num_args = 0;
		if (!cursor_.match(TokenKind::RBracket)) {
			do {
				parse_subexpression();
				++num_args;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RBracket, Message::ExpectBracketAfterIndex);
		}
		return num_args;
	}

	Ast::NodeIndex on_break() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstBreakExpr {token.offset});

		bool has_expression = parse_optional_subexpression();

		auto& break_expr = ast_.get(node_idx).as<AstBreakExpr>();
		break_expr.has_label = has_expression;

		return node_idx;
	}

	Ast::NodeIndex on_continue() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstContinueExpr {token.offset});

		bool has_expression = parse_optional_subexpression();

		auto& continue_expr = ast_.get(node_idx).as<AstContinueExpr>();
		continue_expr.has_label = has_expression;

		return node_idx;
	}

	Ast::NodeIndex on_return() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstReturnExpr {token.offset});

		uint16_t num_expressions = 0;
		if (expression_may_follow()) {
			do {
				parse_subexpression();
				++num_expressions;
			} while (cursor_.match(TokenKind::Comma));
		}

		auto& return_expr = ast_.get(node_idx).as<AstReturnExpr>();
		return_expr.num_expressions = num_expressions;

		return node_idx;
	}

	Ast::NodeIndex on_throw() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstThrowExpr {token.offset});

		bool has_expression = parse_optional_subexpression();

		auto& throw_expr = ast_.get(node_idx).as<AstThrowExpr>();
		throw_expr.has_expression = has_expression;

		return node_idx;
	}

	bool parse_optional_subexpression() {
		if (expression_may_follow()) {
			parse_subexpression();
			return true;
		} else {
			return false;
		}
	}

	bool expression_may_follow() {
		auto next = cursor_.peek_kind();
		return lookup_head_parse_method(next) != nullptr;
	}

	template<UnaryOperator O>
	Ast::NodeIndex on_prefix_operator() {
		auto token = cursor_.next();
		auto node_idx = ast_.store(AstUnaryExpr {token.offset, O});

		parse_subexpression(Precedence::Prefix);

		return node_idx;
	}

	template<BinaryOperator O>
	void on_binary_operator(Ast::NodeIndex left, SourceOffset offset) {
		static constexpr auto precedence = lookup_precedence_for_associativity(O);

		auto operator_token = cursor_.next();
		if constexpr (O == BinaryOperator::Shr) {
			cursor_.advance(); // consume second right angle bracket since `>>` is lexed as two tokens
		}

		if constexpr (O == BinaryOperator::Pow) {
			check_negation_exponentiation_ambiguity(left, operator_token);
		}

		if (auto left_expr = ast_.get(left).get<AstBinaryExpr>()) {
			check_binary_operator_ambiguity(left_expr->op, O, operator_token);
		}

		ast_.store_parent_of(left, AstBinaryExpr {offset, O});
		auto right = parse_subexpression(precedence);

		if (auto right_expr = ast_.get(right).get<AstBinaryExpr>()) {
			check_binary_operator_ambiguity(O, right_expr->op, operator_token);
		}
	}

	void check_binary_operator_ambiguity(BinaryOperator left, BinaryOperator right, Token operator_token) {
		if (operators_are_ambiguous(left, right)) {
			auto location = operator_token.locate_in(source_);
			auto left_str = binary_operator_to_string(left);
			auto right_str = binary_operator_to_string(right);
			report(Message::AmbiguousOperatorMixing, location, MessageArgs(left_str, right_str));
		}
	}

	static bool operators_are_ambiguous(BinaryOperator left, BinaryOperator right) {
		using enum BinaryOperator;

		static constexpr BinaryOperator bitwise_operators[] {BitAnd, BitOr, Xor, Shl, Shr};
		static constexpr BinaryOperator arithmetic_operators[] {Add, Sub, Mul, Div, Rem, Pow};
		static constexpr BinaryOperator comparison_operators[] {Eq, NotEq, Less, Greater, LessEq, GreaterEq};

		struct OperatorPair {
			BinaryOperator left, right;
			bool operator==(const OperatorPair&) const = default;
		};

		static constexpr OperatorPair transitive_comparisons[] {{Eq, Eq},
																{Less, Less},
																{Less, LessEq},
																{LessEq, LessEq},
																{LessEq, Less},
																{Greater, Greater},
																{Greater, GreaterEq},
																{GreaterEq, GreaterEq},
																{GreaterEq, Greater}};

		switch (left) {
			case Add:
			case Sub:
			case Mul:
			case Div:
			case Rem:
			case Pow:		return contains(bitwise_operators, right);
			case BitAnd:
			case BitOr:
			case Xor:
			case Shl:
			case Shr:		return contains(arithmetic_operators, right);
			case LogicAnd:	return right == LogicOr;
			case LogicOr:	return right == LogicAnd;
			case Eq:
			case NotEq:
			case Less:
			case Greater:
			case LessEq:
			case GreaterEq: return contains(comparison_operators, right) && !contains(transitive_comparisons, {left, right});
			default:		return false;
		}
	}

	void check_negation_exponentiation_ambiguity(Ast::NodeIndex left, Token operator_token) {
		if (auto unary = ast_.get(left).get<AstUnaryExpr>()) {
			if (unary->op == UnaryOperator::Neg) {
				auto location = operator_token.locate_in(source_);
				report(Message::AmbiguousOperatorMixing, location, MessageArgs("-", "**"));
			}
		}
	}

	template<UnaryOperator O>
	void on_postfix_operator(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		ast_.store_parent_of(left, AstUnaryExpr {offset, O});
	}

	void on_dot(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		auto member = expect_name(Message::ExpectNameAfterDot);
		ast_.store_parent_of(left, AstMemberExpr {offset, member, 0}); // TODO: generic members
	}

	void on_infix_left_paren(Ast::NodeIndex left, SourceOffset offset) {
		ScopedAssign _(open_angles_, 0);
		cursor_.advance();

		auto node_idx = ast_.store_parent_of(left, AstCallExpr {offset});

		uint16_t num_args = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_subexpression();
				++num_args;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectClosingParen);
		}

		auto& call_expr = ast_.get(node_idx).as<AstCallExpr>();
		call_expr.num_args = num_args;
	}

	void on_infix_left_bracket(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		auto node_idx = ast_.store_parent_of(left, AstIndexExpr {offset});

		auto num_args = parse_bracketed_arguments();

		auto& index_expr = ast_.get(node_idx).as<AstIndexExpr>();
		index_expr.num_args = num_args;
	}

	Ast::NodeIndex on_caret() {
		auto token = cursor_.next();
		return parse_pointer_type(token.offset);
	}

	Ast::NodeIndex on_permission() {
		auto token = cursor_.next();
		return parse_permission(token.offset);
	}

	Ast::NodeIndex parse_permission(SourceOffset offset) {
		auto node_idx = ast_.store(AstPermissionExpr {offset});

		auto specifier = PermissionSpecifier::Var;
		uint16_t num_args = 0;
		if (cursor_.match(TokenKind::LBrace)) {
			ScopedAssign _(open_angles_, 0);

			specifier = PermissionSpecifier::VarBounded;
			if (!cursor_.match(TokenKind::RBrace)) {
				do {
					parse_subexpression();
					++num_args;
				} while (cursor_.match(TokenKind::Comma));

				if (cursor_.match(TokenKind::Ellipsis)) {
					specifier = PermissionSpecifier::VarUnbounded;
				}

				expect(TokenKind::RBrace, Message::ExpectBraceAfterPermission);
			}
		}

		auto& permission_expr = ast_.get(node_idx).as<AstPermissionExpr>();
		permission_expr.specifier = specifier;
		permission_expr.num_args = num_args;

		return node_idx;
	}

	Ast::NodeIndex parse_type() {
		auto offset = cursor_.peek_offset();

		if (cursor_.match(TokenKind::Caret)) {
			return parse_pointer_type(offset);
		}
		if (cursor_.match(TokenKind::LBracket)) {
			return parse_array_type(offset);
		}
		if (cursor_.match(TokenKind::LParen)) {
			return parse_function_type(offset);
		}

		auto name = expect_name(Message::ExpectType);
		return parse_name(offset, name);
	}

	Ast::NodeIndex parse_array_type(SourceOffset offset) {
		auto node_idx = ast_.store(AstArrayTypeExpr {offset});

		bool has_bound = !cursor_.match(TokenKind::RBracket);
		if (has_bound) {
			parse_subexpression();
			expect(TokenKind::RBracket, Message::ExpectBracketAfterArrayBound);
		}
		parse_type();

		auto& array_type_expr = ast_.get(node_idx).as<AstArrayTypeExpr>();
		array_type_expr.has_bound = has_bound;

		return node_idx;
	}

	Ast::NodeIndex parse_pointer_type(SourceOffset offset) {
		auto node_idx = ast_.store(AstPointerTypeExpr {offset});

		bool has_permission = cursor_.peek_kind() == TokenKind::Var;
		if (has_permission) { // TODO: handle other ways to have subexpressions here
			parse_subexpression();
		}
		parse_type();

		auto& ptr_type_expr = ast_.get(node_idx).as<AstPointerTypeExpr>();
		ptr_type_expr.has_permission = has_permission;

		return node_idx;
	}

	Ast::NodeIndex parse_function_type(SourceOffset offset) {
		auto node_idx = ast_.store(AstFunctionTypeExpr {offset});

		auto num_parameters = parse_function_type_parameters();
		expect(TokenKind::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto num_outputs = parse_function_type_outputs();

		auto& func_type_expr = ast_.get(node_idx).as<AstFunctionTypeExpr>();
		func_type_expr.num_parameters = num_parameters;
		func_type_expr.num_outputs = num_outputs;

		return node_idx;
	}

	uint16_t parse_function_type_parameters() {
		uint16_t num_parameters = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_function_type_parameter();
				++num_parameters;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectParenAfterParams);
		}
		return num_parameters;
	}

	void parse_function_type_parameter() {
		auto offset = cursor_.peek_offset();

		auto specifier = ParameterSpecifier::None;
		if (cursor_.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor_.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto node_idx = ast_.store(AstFunctionParameter {offset, specifier, {}, false});

		parse_type();
		auto name = cursor_.match_name(source_);

		auto& param = ast_.get(node_idx).as<AstFunctionParameter>();
		param.name = name;

		if (auto equal = cursor_.match_token(TokenKind::Eq)) {
			auto location = equal->locate_in(source_);
			report(Message::FuncTypeDefaultArgument, location, {});
			throw ParseError();
		}
	}

	uint16_t parse_function_type_outputs() {
		uint16_t num_outputs = 0;
		do {
			parse_function_type_output();
			++num_outputs;
		} while (cursor_.match(TokenKind::Comma));
		expect(TokenKind::RParen, Message::ExpectParenAfterOutputs);

		return num_outputs;
	}

	void parse_function_type_output() {
		auto offset = cursor_.peek_offset();
		auto node_idx = ast_.store(AstFunctionOutput {offset});

		parse_type();
		auto name = cursor_.match_name(source_);

		auto& output = ast_.get(node_idx).as<AstFunctionOutput>();
		output.name = name;
	}

	void expect(TokenKind kind, Message message) {
		if (!cursor_.match(kind)) {
			report_expectation(message);
			throw ParseError();
		}
	}

	std::string_view expect_name(Message message) {
		auto name = cursor_.match_name(source_);
		if (name.empty()) {
			report_expectation(message);
		}

		return name;
	}

	void report_expectation(Message message) {
		auto token = cursor_.peek();
		auto location = token.locate_in(source_);

		auto format = get_token_message_format(token.kind);
		auto lexeme = cursor_.get_lexeme(source_);
		auto message_str = fmt::vformat(format, fmt::make_format_args(lexeme));

		report(message, location, MessageArgs(message_str));
	}

	void report(Message message, CodeLocation location, MessageArgs args) {
		if (!is_looking_ahead_) {
			reporter_.report(message, location, std::move(args));
			ast_.has_errors_ = true;
		}
	}
};

Ast parse(const SourceGuard& source, Reporter& reporter) {
	auto token_stream = lex(source, reporter, false);
	return parse(token_stream, source, reporter);
}

Ast parse(const TokenStream& token_stream, const SourceGuard& source, Reporter& reporter) {
	return Parser(token_stream, source, reporter).parse();
}

} // namespace cero
