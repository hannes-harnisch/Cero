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
		auto& root = ast_.store<AstRoot>();

		uint16_t num_definitions = 0;
		while (!cursor_.match(TokenKind::EndOfFile)) {
			try {
				parse_definition();
				++num_definitions;
			} catch (ParseError) {
				recover_at_definition_scope();
			}
		}

		root = AstRoot(AstNodeHeader<AstNodeKind::Root>(0), num_definitions);
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

	/// Parse method for grammar rules determined by the kind of their first token, like prefix operators
	using HeadParseMethod = Ast::NodeIndex (Parser::*)();

	/// Parse method for grammar rules determined by the kind of token appearing after another expression, like infix operators
	using TailParseMethod = void (Parser::*)(Ast::NodeIndex head_begin, SourceOffset offset);

	struct TailParseRule {
		Precedence precedence = {};
		TailParseMethod method = nullptr;
	};

	void parse_definition() {
		auto offset = cursor_.peek_offset();

		auto access_specifier = AccessSpecifier::None;
		if (cursor_.match(TokenKind::Private)) {
			access_specifier = AccessSpecifier::Private;
		} else if (cursor_.match(TokenKind::Public)) {
			access_specifier = AccessSpecifier::Public;
		}

		auto name = cursor_.match_identifier(source_);
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
			kind = cursor_.current_kind();
		} while (!contains(recovery_tokens, kind));
	}

	void parse_struct(SourceOffset offset, AccessSpecifier access_specifier) {
		auto& struct_def = ast_.store<AstStructDefinition>();

		auto name = expect_name(Message::ExpectNameForStruct);

		struct_def = AstStructDefinition {offset, access_specifier, name};
	}

	void parse_enum(SourceOffset offset, AccessSpecifier access_specifier) {
		auto& enum_def = ast_.store<AstEnumDefinition>();

		auto name = expect_name(Message::ExpectNameForEnum);

		enum_def = AstEnumDefinition {offset, access_specifier, name};
	}

	void parse_function(SourceOffset offset, AccessSpecifier access_specifier, StringId name) {
		auto& func_def = ast_.store<AstFunctionDefinition>();

		expect(TokenKind::LParen, Message::ExpectParenAfterFuncName);

		auto num_parameters = parse_function_definition_parameters();
		auto num_outputs = parse_function_definition_outputs();
		expect(TokenKind::LBrace, Message::ExpectBraceBeforeFuncBody);

		auto num_statements = parse_block();
		func_def = AstFunctionDefinition {offset, access_specifier, name, num_parameters, num_outputs, num_statements};
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
		auto& parameter = ast_.store<AstFunctionParameter>();

		auto offset = cursor_.peek_offset();

		auto specifier = ParameterSpecifier::None;
		if (cursor_.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor_.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		parse_type();
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty()) {
			throw ParseError(); // TODO: test if this is really necessary
		}

		bool has_default_argument = false;
		if (cursor_.match(TokenKind::Eq)) {
			parse_subexpression();
			has_default_argument = true;
		}

		parameter = AstFunctionParameter(offset, specifier, name, has_default_argument);
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
		auto& output = ast_.store<AstFunctionOutput>();
		auto offset = cursor_.peek_offset();

		parse_type();
		auto name = cursor_.match_identifier(source_);

		output = AstFunctionOutput(offset, name);
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
		TokenKind kind = cursor_.current_kind();
		while (kind != TokenKind::EndOfFile) {
			if (kind == TokenKind::Semicolon) {
				cursor_.advance();
				return false;
			}
			if (kind == TokenKind::RBrace) {
				return false;
			}

			cursor_.advance();
			kind = cursor_.current_kind();
		}
		return true;
	}

	void parse_statement() {
		bool parses_complete_stmt = false;
		auto parse_method = lookup_statement_parse_method(parses_complete_stmt);

		auto offset = cursor_.peek_offset();
		auto prev_expr = (this->*parse_method)();

		if (!parses_complete_stmt) {
			auto name_offset = cursor_.peek_offset();
			auto name = cursor_.match_identifier(source_);
			if (!name.empty()) {
				on_trailing_name(offset, prev_expr, name, name_offset);
			}
			expect(TokenKind::Semicolon, Message::ExpectSemicolon);
		}
	}

	HeadParseMethod lookup_statement_parse_method(bool& parses_complete_stmt) {
		auto next = cursor_.peek_kind();
		switch (next) {
			using enum TokenKind;
			case If:	 parses_complete_stmt = true; return &Parser::on_if_stmt;
			case For:	 parses_complete_stmt = true; return &Parser::on_for;
			case While:	 parses_complete_stmt = true; return &Parser::on_while;
			case LBrace: parses_complete_stmt = true; return &Parser::on_left_brace;
			case Let:	 return &Parser::on_let;
			case Var:	 return &Parser::on_var;
			case Const:	 return &Parser::on_const;
			case Static: return &Parser::on_static;
			default:	 return &Parser::parse_expression_or_binding;
		}
	}

	void on_trailing_name(SourceOffset offset, Ast::NodeIndex prev_expr, StringId name, SourceOffset name_offset) {
		auto kind = nodes_[prev_expr].get_kind();

		static constexpr AstNodeKind type_expr_kinds[] {AstNodeKind::NameExpr,		  AstNodeKind::GenericNameExpr,
														AstNodeKind::MemberExpr,	  AstNodeKind::ArrayTypeExpr,
														AstNodeKind::PointerTypeExpr, AstNodeKind::FunctionTypeExpr};
		if (!contains(type_expr_kinds, kind)) {
			auto location = source_.locate(name_offset);
			report(Message::NameCannotAppearHere, location, {});
			throw ParseError();
		}

		bool has_initializer = false;
		if (cursor_.match(TokenKind::Eq)) {
			parse_subexpression();
			has_initializer = true;
		}

		ast_.insert_parent(prev_expr, AstBindingStatement {offset, BindingSpecifier::Let, true, name, has_initializer});
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
						rule = {Precedence::AdditiveOrBitwise, &Parser::on_binary_operator<BinaryOperator::Shr>};
					} else {
						rule = {Precedence::Comparison, &Parser::on_binary_operator<BinaryOperator::Greater>};
					}
				}
				break;

			default: rule = {}; break;
		}

		if (current_precedence < rule.precedence) {
			return rule.method;
		} else {
			return nullptr;
		}
	}

	Ast::NodeIndex on_if_stmt() {
		auto token = cursor_.next();

		auto if_stmt_begin = parse_expression_or_binding();
		expect_colon_or_block();
		parse_statement();

		bool has_else = false;
		if (cursor_.match(TokenKind::Else)) {
			parse_statement();
			has_else = true;
		}

		ast_.insert_parent(if_stmt_begin, AstIfExpr {token.offset, has_else});
		return if_stmt_begin;
	}

	Ast::NodeIndex on_if_expr() {
		auto token = cursor_.next();

		auto if_expr_begin = parse_expression_or_binding();
		expect(TokenKind::Colon, Message::ExpectColonInIfExpr);
		parse_subexpression();
		expect(TokenKind::Else, Message::ExpectElse);
		parse_subexpression();

		ast_.insert_parent(if_expr_begin, AstIfExpr {token.offset, true});
		return if_expr_begin;
	}

	Ast::NodeIndex on_while() {
		auto token = cursor_.next();

		auto while_begin = parse_expression_or_binding();
		expect_colon_or_block();
		cursor_.advance(); // TODO: cleanup depending on whether blocks are required here
		auto num_statements = parse_block();

		ast_.insert_parent(while_begin, AstWhileLoop {token.offset, num_statements});
		return while_begin;
	}

	Ast::NodeIndex on_for() {
		cursor_.advance();

		to_do();
	}

	Ast::NodeIndex on_left_brace() {
		auto token = cursor_.next();

		auto block_stmt_begin = ast_.next_index();
		auto num_statements = parse_block();
		ast_.insert_parent(block_stmt_begin, AstBlockStatement {token.offset, num_statements});
		return block_stmt_begin;
	}

	/// True if colon is matched.
	bool expect_colon_or_block() {
		if (auto colon = cursor_.match_token(TokenKind::Colon)) {
			auto next = cursor_.peek();
			if (next.kind == TokenKind::LBrace) {
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source_), {});
			}
			return true;
		} else {
			auto next = cursor_.peek();
			if (next.kind != TokenKind::LBrace) {
				report_expectation(Message::ExpectColonOrBlock);
			}
			return false;
		}
	}

	Ast::NodeIndex on_let() {
		auto token = cursor_.next();

		auto let_stmt_begin = ast_.next_index();
		auto name = expect_name(Message::ExpectNameAfterLet);

		bool has_initializer = false;
		if (cursor_.match(TokenKind::Eq)) {
			let_stmt_begin = parse_subexpression();
			has_initializer = true;
		}
		ast_.insert_parent(let_stmt_begin,
						   AstBindingStatement {token.offset, BindingSpecifier::Let, false, name, has_initializer});
		return let_stmt_begin;
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
		auto lookahead = cursor_;
		auto name = lookahead.match_identifier(source_);
		if (!name.empty()) {
			if (lookahead.match(TokenKind::Eq)) {
				cursor_ = lookahead;

				auto binding_begin = parse_subexpression();

				ast_.insert_parent(binding_begin, AstBindingStatement {offset, specifier, false, name, true});
				return binding_begin;
			}
		}

		auto binding_begin = parse_type();
		name = expect_name(Message::ExpectNameAfterDeclarationType);

		bool has_initializer = false;
		if (cursor_.match(TokenKind::Eq)) {
			parse_subexpression();
			has_initializer = true;
		}

		ast_.insert_parent(binding_begin, AstBindingStatement {offset, specifier, true, name, has_initializer});
		return binding_begin;
	}

	Ast::NodeIndex on_name() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();
		return parse_name(token.offset, lexeme);
	}

	Ast::NodeIndex parse_name(SourceOffset offset, StringId name) {
		auto saved_cursor = cursor_;
		if (cursor_.match(TokenKind::LAngle)) {
			return parse_generic_name(offset, name, saved_cursor);
		}

		auto name_begin = ast_.next_index();
		nodes_.push_back(AstNameExpr {offset, name});
		return name_begin;
	}

	Ast::NodeIndex parse_generic_name(SourceOffset offset, StringId name, TokenCursor name_start) {
		ScopedAssign _(open_angles_, open_angles_ + 1);

		const auto name_begin = ast_.next_index();
		uint16_t num_generic_args = 0;
		if (!cursor_.match(TokenKind::RAngle)) {
			const bool fall_back = should_fall_back_to_name();

			cursor_ = name_start;
			rescind_lookahead(name_begin);
			if (fall_back) {
				nodes_.push_back(AstNameExpr {offset, name});
				return name_begin;
			}

			cursor_.advance();
			do {
				parse_subexpression();
				++num_generic_args;
			} while (cursor_.match(TokenKind::Comma));
			cursor_.advance();
		}

		ast_.insert_parent(name_begin, AstGenericNameExpr {offset, name, num_generic_args});
		return name_begin;
	}

	bool should_fall_back_to_name() {
		ScopedAssign _(is_looking_ahead_, true);

		do {
			parse_subexpression();
		} while (cursor_.match(TokenKind::Comma));

		static constexpr TokenKind fallbacks[] {TokenKind::DecIntLiteral, TokenKind::HexIntLiteral, TokenKind::BinIntLiteral,
												TokenKind::OctIntLiteral, TokenKind::FloatLiteral,	TokenKind::CharLiteral,
												TokenKind::StringLiteral, TokenKind::Minus,			TokenKind::Tilde,
												TokenKind::Amp,			  TokenKind::PlusPlus,		TokenKind::MinusMinus};
		if (cursor_.match(TokenKind::RAngle)) {
			auto kind = cursor_.peek_kind();
			return (kind == TokenKind::Name && !is_binding_allowed_) || contains(fallbacks, kind)
				   || (open_angles_ == 1 && kind == TokenKind::RAngle);
		}
		return true;
	}

	template<void LiteralParseFn(std::string_view), NumericLiteralKind Kind>
	Ast::NodeIndex on_numeric_literal() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();
		LiteralParseFn(lexeme);

		auto literal_begin = ast_.next_index();
		nodes_.push_back(AstNumericLiteralExpr {token.offset, Kind});
		return literal_begin;
	}

	Ast::NodeIndex on_string_literal() {
		auto lexeme = cursor_.get_lexeme(source_);
		auto token = cursor_.next();

		auto literal_begin = ast_.next_index();
		nodes_.push_back(AstStringLiteralExpr {token.offset, evaluate_string_literal(lexeme)});
		return literal_begin;
	}

	Ast::NodeIndex on_prefix_left_paren() { // TODO: function type
		ScopedAssign _(open_angles_, 0);

		auto token = cursor_.next();
		auto group_begin = ast_.next_index();

		uint16_t num_args = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_subexpression();
				++num_args;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectClosingParen);
		}

		ast_.insert_parent(group_begin, AstGroupExpr {token.offset, num_args});
		return group_begin;
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

		bool has_expression = false;
		auto break_begin = parse_optional_subexpression(has_expression);
		ast_.insert_parent(break_begin, AstBreakExpr {token.offset, has_expression});
		return break_begin;
	}

	Ast::NodeIndex on_continue() {
		auto token = cursor_.next();

		bool has_expression = false;
		auto continue_begin = parse_optional_subexpression(has_expression);
		ast_.insert_parent(continue_begin, AstContinueExpr {token.offset, has_expression});
		return continue_begin;
	}

	Ast::NodeIndex on_return() {
		auto token = cursor_.next();

		auto return_begin = ast_.next_index();
		uint16_t num_expressions = 0;
		if (expression_may_follow()) {
			do {
				parse_subexpression();
				++num_expressions;
			} while (cursor_.match(TokenKind::Comma));
		}

		ast_.insert_parent(return_begin, AstReturnExpr {token.offset, num_expressions});
		return return_begin;
	}

	Ast::NodeIndex on_throw() {
		auto token = cursor_.next();

		bool has_expression = false;
		auto throw_begin = parse_optional_subexpression(has_expression);
		ast_.insert_parent(throw_begin, AstThrowExpr {token.offset, has_expression});
		return throw_begin;
	}

	Ast::NodeIndex parse_optional_subexpression(bool& has_expression) {
		if (expression_may_follow()) {
			has_expression = true;
			return parse_subexpression();
		} else {
			return ast_.next_index();
		}
	}

	bool expression_may_follow() {
		auto next = cursor_.peek_kind();
		return lookup_head_parse_method(next) != nullptr;
	}

	template<UnaryOperator O>
	Ast::NodeIndex on_prefix_operator() {
		auto token = cursor_.next();

		auto expr_begin = parse_subexpression(Precedence::Prefix);

		ast_.insert_parent(expr_begin, AstUnaryExpr {token.offset, O});
		return expr_begin;
	}

	template<BinaryOperator O>
	void on_binary_operator(Ast::NodeIndex left, SourceOffset offset) {
		static constexpr auto precedence = lookup_precedence_for_associativity(O);

		auto operator_token = cursor_.next();
		if constexpr (O == BinaryOperator::Shr) {
			cursor_.advance();
		}

		auto right = parse_subexpression(precedence);
		validate_associativity(O, left, right, operator_token);

		ast_.insert_parent(left, AstBinaryExpr {offset, O});
	}

	void validate_associativity(BinaryOperator op, Ast::NodeIndex left_idx, Ast::NodeIndex right_idx, Token operator_token) {
		auto& left_node = nodes_[left_idx];
		auto& right_node = nodes_[right_idx];

		if (auto right = right_node.get<AstBinaryExpr>()) {
			validate_binary_associativity(op, right->op, operator_token);
		}

		if (auto left = left_node.get<AstBinaryExpr>()) {
			validate_binary_associativity(left->op, op, operator_token);
		} else if (auto unary = left_node.get<AstUnaryExpr>()) {
			validate_unary_binary_associativity(unary->op, op, operator_token);
		}
	}

	void validate_binary_associativity(BinaryOperator left, BinaryOperator right, Token operator_token) {
		if (associates_ambiguous_operators(left, right)) {
			auto location = operator_token.locate_in(source_);
			auto left_str = binary_operator_to_string(left);
			auto right_str = binary_operator_to_string(right);
			report(Message::AmbiguousOperatorMixing, location, MessageArgs(left_str, right_str));
		}
	}

	static bool associates_ambiguous_operators(BinaryOperator left, BinaryOperator right) {
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

	void validate_unary_binary_associativity(UnaryOperator left, BinaryOperator right, Token operator_token) {
		if (left == UnaryOperator::Neg && right == BinaryOperator::Pow) {
			auto location = operator_token.locate_in(source_);
			report(Message::AmbiguousOperatorMixing, location, MessageArgs("-", "**"));
		}
	}

	template<UnaryOperator O>
	void on_postfix_operator(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		ast_.insert_parent(left, AstUnaryExpr {offset, O});
	}

	void on_dot(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		auto member = expect_name(Message::ExpectNameAfterDot);
		ast_.insert_parent(left, AstMemberExpr {offset, member, 0});
	}

	void on_infix_left_paren(Ast::NodeIndex left, SourceOffset offset) {
		ScopedAssign _(open_angles_, 0);

		cursor_.advance();
		uint16_t num_args = 0;
		if (!cursor_.match(TokenKind::RParen)) {
			do {
				parse_subexpression();
				++num_args;
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RParen, Message::ExpectClosingParen);
		}
		ast_.insert_parent(left, AstCallExpr {offset, num_args});
	}

	void on_infix_left_bracket(Ast::NodeIndex left, SourceOffset offset) {
		cursor_.advance();

		auto num_args = parse_bracketed_arguments();
		ast_.insert_parent(left, AstIndexExpr {offset, num_args});
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
		auto permission_begin = ast_.next_index();

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

		ast_.insert_parent(permission_begin, AstPermissionExpr {offset, specifier, num_args});
		return permission_begin;
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
		bool has_bound;
		Ast::NodeIndex array_type_begin;
		if (cursor_.match(TokenKind::RBracket)) {
			has_bound = false;
			array_type_begin = parse_type();
		} else {
			has_bound = true;
			array_type_begin = parse_subexpression();
			expect(TokenKind::RBracket, Message::ExpectBracketAfterArrayBound);
			parse_type();
		}

		ast_.insert_parent(array_type_begin, AstArrayTypeExpr {offset, has_bound});
		return array_type_begin;
	}

	Ast::NodeIndex parse_pointer_type(SourceOffset offset) {
		bool has_permission;
		Ast::NodeIndex ptr_type_begin;
		if (cursor_.peek_kind() == TokenKind::Var) { // TODO: handle other ways to have subexpressions here
			has_permission = true;
			ptr_type_begin = parse_subexpression();
			parse_type();
		} else {
			has_permission = false;
			ptr_type_begin = parse_type();
		}

		ast_.insert_parent(ptr_type_begin, AstPointerTypeExpr {offset, has_permission});
		return ptr_type_begin;
	}

	Ast::NodeIndex parse_function_type(SourceOffset offset) {
		auto func_type_begin = ast_.next_index();

		auto num_parameters = parse_function_type_parameters();
		expect(TokenKind::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto num_outputs = parse_function_type_outputs();

		ast_.insert_parent(func_type_begin, AstFunctionTypeExpr {offset, num_parameters, num_outputs});
		return func_type_begin;
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

		auto param_begin = parse_type();
		auto name = cursor_.match_identifier(source_);

		if (auto equal = cursor_.match_token(TokenKind::Eq)) {
			auto location = equal->locate_in(source_);
			report(Message::FuncTypeDefaultArgument, location, {});
			throw ParseError();
		}

		ast_.insert_parent(param_begin, AstFunctionParameter {offset, specifier, name, false});
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

		auto output_begin = parse_type();
		auto name = cursor_.match_identifier(source_);

		ast_.insert_parent(output_begin, AstFunctionOutput {offset, name});
	}

	void rescind_lookahead(Ast::NodeIndex node_index) {
		auto first = nodes_.begin() + static_cast<ptrdiff_t>(node_index);
		nodes_.erase(first, nodes_.end());
	}

	void expect(TokenKind kind, Message message) {
		if (!cursor_.match(kind)) {
			report_expectation(message);
			throw ParseError();
		}
	}

	std::string_view expect_name(Message message) {
		auto name = cursor_.match_identifier(source_);
		if (name.empty()) {
			report_expectation(message);
		}

		return name;
	}

	void report_expectation(Message message) {
		auto token = cursor_.current();
		auto location = token.locate_in(source_);

		auto format = get_token_message_format(token.kind);
		auto lexeme = cursor_.get_lexeme(source_);
		auto message_str = fmt::vformat(format, fmt::make_format_args(lexeme));

		report(message, location, MessageArgs(message_str));
	}

	void report(Message message, CodeLocation location, MessageArgs args) {
		if (!is_looking_ahead_) {
			reporter_.report(message, location, std::move(args));
		}
	}
};

Ast parse(const SourceGuard& source, Reporter& reporter) {
	auto token_stream = lex(source, reporter);
	return parse(token_stream, source, reporter);
}

Ast parse(const TokenStream& token_stream, const SourceGuard& source, Reporter& reporter) {
	return Parser(token_stream, source, reporter).parse();
}

} // namespace cero
