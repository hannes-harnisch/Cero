#include "Parse.hpp"

#include "syntax/Lex.hpp"
#include "syntax/Literal.hpp"
#include "syntax/ParseCursor.hpp"
#include "util/Algorithm.hpp"
#include "util/Defer.hpp"
#include "util/Fail.hpp"

namespace cero {

enum class Precedence : uint8_t {
	Statement,
	Assignment,
	Logical,
	Comparison,
	Bitwise,
	Multiplicative,
	Prefix,
	Postfix
};

struct ParseError {};

class Parser {
public:
	Parser(const TokenStream& token_stream, const Source& source, Reporter& reporter) :
		source(source),
		reporter(reporter),
		cursor(token_stream) {
	}

	Ast parse() {
		std::vector<AstNode> definitions;
		while (!cursor.match(TokenKind::EndOfFile)) {
			try {
				definitions.emplace_back(parse_definition());
			} catch (ParseError) {
				recover_at_definition_scope();
			}
		}
		auto defs = ast.store_multiple(definitions);
		ast.store(AstRoot {defs});
		return std::move(ast);
	}

private:
	Ast ast;
	const Source& source;
	Reporter& reporter;
	ParseCursor cursor;
	bool is_looking_ahead = false;
	bool is_binding_allowed = true;
	uint32_t open_angles = 0;

	class Lookahead {
	public:
		Lookahead(Parser& parser) :
			parser(parser),
			restore(true),
			saved_node_count(parser.ast.ast_nodes.size()),
			was_looking_ahead(parser.is_looking_ahead) {
			parser.is_looking_ahead = true;
		}

		~Lookahead() {
			if (restore) {
				parser.rescind_lookahead(saved_node_count);
			}
			parser.is_looking_ahead = was_looking_ahead;
		}

		void accept() {
			restore = false;
		}

		Lookahead(const Lookahead&) = delete;
		Lookahead& operator=(const Lookahead&) = delete;

	private:
		Parser& parser;
		size_t saved_node_count;
		bool restore;
		bool was_looking_ahead;
	};

	using PrefixParse = AstNode (Parser::*)();
	using NonPrefixParse = AstNode (Parser::*)(AstId left);

	struct NonPrefixParseRule {
		Precedence precedence = {};
		NonPrefixParse func = nullptr;
	};

	AstNode parse_definition() {
		auto access_specifier = AccessSpecifier::None;
		if (cursor.match(TokenKind::Private)) {
			access_specifier = AccessSpecifier::Private;
		} else if (cursor.match(TokenKind::Public)) {
			access_specifier = AccessSpecifier::Public;
		}

		if (auto name_token = cursor.match_name()) {
			return parse_function(access_specifier, *name_token);
		}

		if (cursor.match(TokenKind::Struct)) {
			return parse_struct(access_specifier);
		}

		if (cursor.match(TokenKind::Enum)) {
			return parse_enum(access_specifier);
		}

		report_expectation(Message::ExpectFuncStructEnum, cursor.current());
		throw ParseError();
	}

	void recover_at_definition_scope() {
		static constexpr TokenKind recovery_tokens[] {TokenKind::Public, TokenKind::Private, TokenKind::Struct, TokenKind::Enum,
													  TokenKind::EndOfFile};

		TokenKind kind;
		do {
			cursor.advance();
			kind = cursor.current_kind();
		} while (!contains(recovery_tokens, kind));
	}

	AstNode parse_struct(AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForStruct);
		return {AstStructDefinition {access_specifier, name}};
	}

	AstNode parse_enum(AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForEnum);
		return {AstEnumDefinition {access_specifier, name}};
	}

	AstNode parse_function(AccessSpecifier access_specifier, Token name_token) {
		auto name = name_token.get_lexeme(source);
		expect(TokenKind::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs = parse_function_definition_outputs();
		expect(TokenKind::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return {AstFunctionDefinition {
			access_specifier,
			name,
			std::move(parameters),
			std::move(outputs),
			statements,
		}};
	}

	std::vector<AstFunctionDefinition::Parameter> parse_function_definition_parameters() {
		std::vector<AstFunctionDefinition::Parameter> parameters;
		if (!cursor.match(TokenKind::RightParen)) {
			do {
				parameters.emplace_back(parse_function_definition_parameter());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionDefinition::Parameter parse_function_definition_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto type = ast.store(parse_type());
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty()) {
			throw ParseError(); // TODO: explain why this is necessary
		}

		OptionalAstId default_argument;
		if (cursor.match(TokenKind::Equals)) {
			default_argument = ast.store(parse_subexpression());
		}

		return {specifier, type, name, default_argument};
	}

	std::vector<AstFunctionDefinition::Output> parse_function_definition_outputs() {
		std::vector<AstFunctionDefinition::Output> outputs;
		if (cursor.match(TokenKind::ThinArrow)) {
			do {
				outputs.emplace_back(parse_function_definition_output());
			} while (cursor.match(TokenKind::Comma));
		}
		return outputs;
	}

	AstFunctionDefinition::Output parse_function_definition_output() {
		auto type = ast.store(parse_type());

		StringId name;
		if (auto name_token = cursor.match_name()) {
			name = name_token->get_lexeme(source);
		}

		return {type, name};
	}

	AstIdSet parse_block() {
		const uint32_t saved_angles = open_angles;
		const bool saved_binding_allowed = is_binding_allowed;
		open_angles = 0;
		is_binding_allowed = true;

		std::vector<AstNode> statements;
		while (!cursor.match(TokenKind::RightBrace)) {
			try {
				statements.emplace_back(parse_statement());
			} catch (ParseError) {
				bool at_end = recover_at_statement_scope();
				if (at_end) {
					break;
				}
			}
		}

		open_angles = saved_angles;
		is_binding_allowed = saved_binding_allowed;
		return ast.store_multiple(statements);
	}

	bool recover_at_statement_scope() {
		TokenKind kind = cursor.current_kind();
		while (kind != TokenKind::EndOfFile) {
			if (kind == TokenKind::Semicolon) {
				cursor.advance();
				return false;
			}
			if (kind == TokenKind::RightBrace) {
				return false;
			}

			cursor.advance();
			kind = cursor.current_kind();
		}
		return true;
	}

	AstNode parse_statement() {
		bool parses_complete_stmt = false;
		auto parse = lookup_statement_parse(parses_complete_stmt);
		auto stmt = (this->*parse)();

		if (!parses_complete_stmt) {
			if (auto name_token = cursor.match_name()) {
				stmt = on_trailing_name(std::move(stmt), *name_token);
			}
			expect(TokenKind::Semicolon, Message::ExpectSemicolon);
		}
		return stmt;
	}

	PrefixParse lookup_statement_parse(bool& parses_complete_stmt) {
		PrefixParse parse;

		auto next = cursor.peek_kind();
		using enum TokenKind;
		switch (next) {
			case If:
				parses_complete_stmt = true;
				parse = &Parser::on_if_stmt;
				break;
			case For:
				parses_complete_stmt = true;
				parse = &Parser::on_for;
				break;
			case While:
				parses_complete_stmt = true;
				parse = &Parser::on_while;
				break;
			case LeftBrace:
				parses_complete_stmt = true;
				parse = &Parser::on_left_brace;
				break;
			case Let: parse = &Parser::on_let; break;
			case Var:; parse = &Parser::on_var; break;
			case Const: parse = &Parser::on_const; break;
			case Static: parse = &Parser::on_static; break;
			default: return &Parser::parse_expression_or_binding;
		}

		cursor.advance();
		return parse;
	}

	AstNode on_trailing_name(AstNode left_expr, Token name_token) {
		auto type = left_expr.get_type();

		static constexpr AstNodeKind type_expr_kinds[] {AstNodeKind::NameExpr,		  AstNodeKind::GenericNameExpr,
														AstNodeKind::MemberExpr,	  AstNodeKind::GenericMemberExpr,
														AstNodeKind::ArrayTypeExpr,	  AstNodeKind::PointerTypeExpr,
														AstNodeKind::FunctionTypeExpr};
		if (!contains(type_expr_kinds, type)) {
			report_expectation(Message::ExpectSemicolon, name_token);
			throw ParseError();
		}

		auto name = name_token.get_lexeme(source);
		auto left = ast.store(std::move(left_expr));

		OptionalAstId initializer;
		if (cursor.match(TokenKind::Equals)) {
			initializer = ast.store(parse_subexpression());
		}

		return {AstBindingStatement {BindingSpecifier::Let, left, name, initializer}};
	}

	AstNode parse_expression_or_binding() {
		const bool saved_is_binding_allowed = is_binding_allowed;
		is_binding_allowed = true;
		Defer _ = [&] {
			is_binding_allowed = saved_is_binding_allowed;
		};
		return parse_expression(Precedence::Statement);
	}

	AstNode parse_subexpression(Precedence precedence = Precedence::Statement) {
		const bool saved_is_binding_allowed = is_binding_allowed;
		is_binding_allowed = false;
		Defer _ = [&] {
			is_binding_allowed = saved_is_binding_allowed;
		};
		return parse_expression(precedence);
	}

	AstNode parse_expression(Precedence precedence) {
		auto next = cursor.peek();

		auto parse_prefix = lookup_prefix_parse(next.kind);
		if (parse_prefix == nullptr) {
			report_expectation(Message::ExpectExpr, next);
			throw ParseError();
		}

		cursor.advance();
		auto expression = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence)) {
			auto left = ast.store(std::move(expression));
			expression = (this->*parse)(left);
		}

		return expression;
	}

	static PrefixParse lookup_prefix_parse(TokenKind kind) {
		using enum TokenKind;
		using enum UnaryOperator;

		switch (kind) {
			case Name: return &Parser::on_name;
			case If: return &Parser::on_if_expr;
			case Var: return &Parser::on_variability;
			case DecIntLiteral: return &Parser::on_numeric_literal<evaluate_dec_int_literal>;
			case HexIntLiteral: return &Parser::on_numeric_literal<evaluate_hex_int_literal>;
			case BinIntLiteral: return &Parser::on_numeric_literal<evaluate_bin_int_literal>;
			case OctIntLiteral: return &Parser::on_numeric_literal<evaluate_oct_int_literal>;
			case FloatLiteral: return &Parser::on_numeric_literal<evaluate_float_literal>;
			case CharLiteral: return &Parser::on_numeric_literal<evaluate_char_literal>;
			case StringLiteral: return &Parser::on_string_literal;
			case LeftParen: return &Parser::on_prefix_left_paren;
			case LeftBracket: return &Parser::on_prefix_left_bracket;
			case Break: return &Parser::on_break;
			case Continue: return &Parser::on_continue;
			case Return: return &Parser::on_return;
			case Throw: return &Parser::on_throw;
			case Ampersand: return &Parser::on_prefix_operator<AddressOf>;
			case Minus: return &Parser::on_prefix_operator<Negate>;
			case Bang: return &Parser::on_prefix_operator<LogicalNot>;
			case Tilde: return &Parser::on_prefix_operator<BitwiseNot>;
			case PlusPlus: return &Parser::on_prefix_operator<PreIncrement>;
			case MinusMinus: return &Parser::on_prefix_operator<PreDecrement>;
			case Caret: return &Parser::on_caret;
			default: return nullptr;
		}
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence current_precedence) {
		auto token = cursor.peek();

		if (token.kind == TokenKind::RightAngle && open_angles > 0) {
			return nullptr;
		}

		auto rule = select_non_prefix_parse(token);
		if (current_precedence >= rule.precedence) {
			return nullptr;
		}

		cursor.advance();
		return rule.func;
	}

	NonPrefixParseRule select_non_prefix_parse(Token current) {
		if (current.kind != TokenKind::RightAngle) {
			return lookup_non_prefix_parse(current.kind);
		}

		auto next = cursor.peek_ahead();
		if (next.kind == TokenKind::RightAngle && next.offset == current.offset + 1) {
			cursor.advance();
			return {Precedence::Bitwise, &Parser::on_infix_operator<BinaryOperator::RightShift>};
		} else {
			return {Precedence::Comparison, &Parser::on_infix_operator<BinaryOperator::Greater>};
		}
	}

	static NonPrefixParseRule lookup_non_prefix_parse(TokenKind kind) {
		using enum TokenKind;
		using enum Precedence;
		using enum UnaryOperator;
		using enum BinaryOperator;

		switch (kind) {
			case Equals: return {Assignment, &Parser::on_infix_operator<Assign>};
			case PlusEquals: return {Assignment, &Parser::on_infix_operator<AddAssign>};
			case MinusEquals: return {Assignment, &Parser::on_infix_operator<SubtractAssign>};
			case StarEquals: return {Assignment, &Parser::on_infix_operator<MultiplyAssign>};
			case SlashEquals: return {Assignment, &Parser::on_infix_operator<DivideAssign>};
			case PercentEquals: return {Assignment, &Parser::on_infix_operator<RemainderAssign>};
			case StarStarEquals: return {Assignment, &Parser::on_infix_operator<PowerAssign>};
			case AmpersandEquals: return {Assignment, &Parser::on_infix_operator<AndAssign>};
			case PipeEquals: return {Assignment, &Parser::on_infix_operator<OrAssign>};
			case TildeEquals: return {Assignment, &Parser::on_infix_operator<XorAssign>};
			case LeftAngleAngleEquals: return {Assignment, &Parser::on_infix_operator<LeftShiftAssign>};
			case RightAngleAngleEquals: return {Assignment, &Parser::on_infix_operator<RightShiftAssign>};
			case AmpersandAmpersand: return {Logical, &Parser::on_infix_operator<LogicalAnd>};
			case PipePipe: return {Logical, &Parser::on_infix_operator<LogicalOr>};
			case EqualsEquals: return {Comparison, &Parser::on_infix_operator<Equal>};
			case BangEquals: return {Comparison, &Parser::on_infix_operator<NotEqual>};
			case LeftAngle: return {Comparison, &Parser::on_infix_operator<Less>};
			case LeftAngleEquals: return {Comparison, &Parser::on_infix_operator<LessEqual>};
			case RightAngleEquals: return {Comparison, &Parser::on_infix_operator<GreaterEqual>};
			case Plus: return {Bitwise, &Parser::on_infix_operator<Add>};
			case Minus: return {Bitwise, &Parser::on_infix_operator<Subtract>};
			case Ampersand: return {Bitwise, &Parser::on_infix_operator<BitAnd>};
			case Pipe: return {Bitwise, &Parser::on_infix_operator<BitOr>};
			case Tilde: return {Bitwise, &Parser::on_infix_operator<Xor>};
			case LeftAngleAngle: return {Bitwise, &Parser::on_infix_operator<LeftShift>};
			case Star: return {Multiplicative, &Parser::on_infix_operator<Multiply>};
			case Slash: return {Multiplicative, &Parser::on_infix_operator<Divide>};
			case Percent: return {Multiplicative, &Parser::on_infix_operator<Remainder>};
			case StarStar: return {Prefix, &Parser::on_infix_operator<Power>};
			case Caret: return {Postfix, &Parser::on_postfix_operator<Dereference>};
			case PlusPlus: return {Postfix, &Parser::on_postfix_operator<PostIncrement>};
			case MinusMinus: return {Postfix, &Parser::on_postfix_operator<PostDecrement>};
			case Dot: return {Postfix, &Parser::on_dot};
			case LeftParen: return {Postfix, &Parser::on_infix_left_paren};
			case LeftBracket: return {Postfix, &Parser::on_infix_left_bracket};
			default: return {};
		}
	}

	AstNode on_if_stmt() {
		auto condition = ast.store(parse_expression_or_binding());
		expect_colon_or_block();

		auto then_stmt = ast.store(parse_statement());

		OptionalAstId else_stmt;
		if (cursor.match(TokenKind::Else)) {
			else_stmt = ast.store(parse_statement());
		}

		return {AstIfExpr {condition, then_stmt, else_stmt}};
	}

	AstNode on_if_expr() {
		auto condition = ast.store(parse_expression_or_binding());
		expect(TokenKind::Colon, Message::ExpectColonInIfExpr);
		auto then_expr = ast.store(parse_subexpression());
		expect(TokenKind::Else, Message::ExpectElse);
		auto else_expr = ast.store(parse_subexpression());

		return {AstIfExpr {condition, then_expr, else_expr}};
	}

	AstNode on_while() {
		auto condition = ast.store(parse_expression_or_binding());
		expect_colon_or_block();
		auto statement = ast.store(parse_statement());
		return {AstWhileLoop {condition, statement}};
	}

	AstNode on_for() {
		to_do();
	}

	AstNode on_left_brace() {
		auto statements = parse_block();
		return {AstBlockStatement {statements}};
	}

	void expect_colon_or_block() {
		if (auto colon = cursor.match_token(TokenKind::Colon)) {
			auto next = cursor.peek();
			if (next.kind == TokenKind::LeftBrace) {
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source));
			}
		} else {
			auto next = cursor.peek();
			if (next.kind != TokenKind::LeftBrace) {
				report_expectation(Message::ExpectColonOrBlock, next);
			}
		}
	}

	AstNode on_let() {
		auto name = expect_name(Message::ExpectNameAfterLet);

		OptionalAstId initializer;
		if (cursor.match(TokenKind::Equals)) {
			initializer = ast.store(parse_subexpression());
		}

		return {AstBindingStatement {BindingSpecifier::Let, {}, name, initializer}};
	}

	AstNode on_var() {
		if (cursor.peek_kind() == TokenKind::LeftBrace) {
			return {parse_variability()};
		}

		return {parse_binding(BindingSpecifier::Var)};
	}

	AstNode on_const() {
		return {parse_binding(BindingSpecifier::Const)};
	}

	AstNode on_static() {
		auto specifier = BindingSpecifier::Static;
		if (cursor.match(TokenKind::Var)) {
			specifier = BindingSpecifier::StaticVar;
		}

		return {parse_binding(specifier)};
	}

	AstBindingStatement parse_binding(BindingSpecifier specifier) {
		auto lookahead = cursor;
		if (auto name_token = lookahead.match_name()) {
			if (lookahead.match(TokenKind::Equals)) {
				cursor = lookahead;
				auto name = name_token->get_lexeme(source);
				auto initializer = ast.store(parse_subexpression());
				return {specifier, {}, name, initializer};
			}
		}

		auto type = ast.store(parse_type());
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalAstId initializer;
		if (cursor.match(TokenKind::Equals)) {
			initializer = ast.store(parse_subexpression());
		}

		return {specifier, type, name, initializer};
	}

	AstNode on_name() {
		auto name = cursor.previous().get_lexeme(source);
		return parse_name(name);
	}

	AstNode parse_name(std::string_view name) {
		auto saved_cursor = cursor;
		if (cursor.match(TokenKind::LeftAngle)) {
			return parse_generic_name(name, saved_cursor);
		}

		return {AstNameExpr {name}};
	}

	AstNode parse_generic_name(std::string_view name, ParseCursor name_start) {
		++open_angles;
		Defer _ = [&] {
			--open_angles;
		};

		std::vector<AstNode> generic_args;
		if (!cursor.match(TokenKind::RightAngle)) {
			size_t saved_node_count = ast.ast_nodes.size();

			bool fall_back = should_fall_back_to_name();

			cursor = name_start;
			rescind_lookahead(saved_node_count);

			if (fall_back) {
				return {AstNameExpr {name}};
			}

			cursor.advance();
			do {
				generic_args.emplace_back(parse_subexpression());
			} while (cursor.match(TokenKind::Comma));
			cursor.advance();
		}

		auto arguments = ast.store_multiple(generic_args);
		return {AstGenericNameExpr {name, arguments}};
	}

	bool should_fall_back_to_name() {
		bool saved = is_looking_ahead;
		is_looking_ahead = true;
		Defer _ = [&] {
			is_looking_ahead = saved;
		};

		do {
			parse_subexpression();
		} while (cursor.match(TokenKind::Comma));

		static constexpr TokenKind fallbacks[] {TokenKind::DecIntLiteral, TokenKind::HexIntLiteral, TokenKind::BinIntLiteral,
												TokenKind::OctIntLiteral, TokenKind::FloatLiteral,	TokenKind::CharLiteral,
												TokenKind::StringLiteral, TokenKind::Minus,			TokenKind::Tilde,
												TokenKind::Ampersand,	  TokenKind::Bang,			TokenKind::PlusPlus,
												TokenKind::MinusMinus};
		if (cursor.match(TokenKind::RightAngle)) {
			auto kind = cursor.peek_kind();
			return (kind == TokenKind::Name && !is_binding_allowed) || contains(fallbacks, kind)
				   || (open_angles == 1 && kind == TokenKind::RightAngle);
		}
		return true;
	}

	template<AstNumericLiteralExpr (*EVALUATE)(std::string_view)>
	AstNode on_numeric_literal() {
		auto lexeme = cursor.previous().get_lexeme(source);
		return {EVALUATE(lexeme)};
	}

	AstNode on_string_literal() {
		auto lexeme = cursor.previous().get_lexeme(source);
		return {evaluate_string_literal(lexeme)};
	}

	AstNode on_prefix_left_paren() { // TODO: function type
		uint32_t saved = open_angles;
		open_angles = 0;
		Defer _ = [&] {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(TokenKind::RightParen)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectClosingParen);
		}
		return {AstGroupExpr {ast.store_multiple(arguments)}};
	}

	AstNode on_prefix_left_bracket() {
		return parse_array_type(); // TODO: array literal
	}

	AstIdSet parse_bracketed_arguments() {
		uint32_t saved = open_angles;
		open_angles = 0;
		Defer _ = [&] {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(TokenKind::RightBracket)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return ast.store_multiple(arguments);
	}

	AstNode on_break() {
		auto label = parse_optional_subexpression();
		return {AstBreakExpr {label}};
	}

	AstNode on_continue() {
		auto label = parse_optional_subexpression();
		return {AstContinueExpr {label}};
	}

	AstNode on_return() {
		std::vector<AstNode> values;
		if (expression_may_follow()) {
			do {
				values.emplace_back(parse_subexpression());
			} while (cursor.match(TokenKind::Comma));
		}

		auto return_values = ast.store_multiple(values);
		return {AstReturnExpr {return_values}};
	}

	AstNode on_throw() {
		auto expr = parse_optional_subexpression();
		return {AstThrowExpr {expr}};
	}

	OptionalAstId parse_optional_subexpression() {
		if (expression_may_follow()) {
			return ast.store(parse_subexpression());
		}

		return {};
	}

	bool expression_may_follow() {
		auto next = cursor.peek_kind();
		return lookup_prefix_parse(next) != nullptr;
	}

	template<UnaryOperator O>
	AstNode on_prefix_operator() {
		auto right = ast.store(parse_subexpression(Precedence::Prefix));
		return {AstUnaryExpr {O, right}};
	}

	template<BinaryOperator O>
	AstNode on_infix_operator(AstId left) {
		auto target = cursor.previous();
		auto precedence = lookup_precedence_for_associativity(O);
		auto right = ast.store(parse_subexpression(precedence));
		validate_associativity(O, left, right, target);
		return {AstBinaryExpr {O, left, right}};
	}

	template<UnaryOperator O>
	AstNode on_postfix_operator(AstId left) {
		return {AstUnaryExpr {O, left}};
	}

	void validate_associativity(BinaryOperator current, AstId left_id, AstId right_id, Token target) {
		auto& left_node = ast.get(left_id);
		auto& right_node = ast.get(right_id);

		if (auto right = right_node.get<AstBinaryExpr>()) {
			validate_binary_associativity(current, right->op, target);
		}

		if (auto left = left_node.get<AstBinaryExpr>()) {
			validate_binary_associativity(left->op, current, target);
		} else if (auto unary = left_node.get<AstUnaryExpr>()) {
			validate_unary_binary_associativity(unary->op, current, target);
		}
	}

	void validate_binary_associativity(BinaryOperator left, BinaryOperator right, Token target) {
		if (associates_ambiguous_operators(left, right)) {
			auto location = target.locate_in(source);
			report(Message::AmbiguousOperatorMixing, location, to_string(left), to_string(right));
		}
	}

	static bool associates_ambiguous_operators(BinaryOperator left, BinaryOperator right) {
		using enum BinaryOperator;
		static constexpr BinaryOperator bitwise_operators[] {BitAnd, BitOr, Xor, LeftShift, RightShift};
		static constexpr BinaryOperator arithmetic_operators[] {Add, Subtract, Multiply, Divide, Remainder, Power};
		static constexpr BinaryOperator comparison_operators[] {Equal, NotEqual, Less, Greater, LessEqual, GreaterEqual};

		struct OperatorPair {
			BinaryOperator left, right;
			bool operator==(const OperatorPair&) const = default;
		};

		static constexpr OperatorPair transitive_comparisons[] {{Equal, Equal},			 {Less, Less},
																{Less, LessEqual},		 {LessEqual, LessEqual},
																{LessEqual, Less},		 {Greater, Greater},
																{Greater, GreaterEqual}, {GreaterEqual, GreaterEqual},
																{GreaterEqual, Greater}};
		switch (left) {
			case Add:
			case Subtract:
			case Multiply:
			case Divide:
			case Remainder:
			case Power: return contains(bitwise_operators, right);
			case BitAnd:
			case BitOr:
			case Xor:
			case LeftShift:
			case RightShift: return contains(arithmetic_operators, right);
			case LogicalAnd: return right == LogicalOr;
			case LogicalOr: return right == LogicalAnd;
			case Equal:
			case NotEqual:
			case Less:
			case Greater:
			case LessEqual:
			case GreaterEqual: return contains(comparison_operators, right) && !contains(transitive_comparisons, {left, right});
			default: return false;
		}
	}

	void validate_unary_binary_associativity(UnaryOperator left, BinaryOperator right, Token target) {
		if (left == UnaryOperator::Negate && right == BinaryOperator::Power) {
			auto location = target.locate_in(source);
			report(Message::AmbiguousOperatorMixing, location, "-", "**");
		}
	}

	AstNode on_dot(AstId left) {
		auto member = expect_name(Message::ExpectNameAfterDot);
		return {AstMemberExpr {left, member}};
	}

	AstNode on_infix_left_paren(AstId left) {
		uint32_t saved = open_angles;
		open_angles = 0;
		Defer _ = [&] {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(TokenKind::RightParen)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectClosingParen);
		}
		return {AstCallExpr {left, ast.store_multiple(arguments)}};
	}

	AstNode on_infix_left_bracket(AstId left) {
		return {AstIndexExpr {left, parse_bracketed_arguments()}};
	}

	AstNode on_caret() {
		return parse_pointer_type();
	}

	AstNode on_variability() {
		return {parse_variability()};
	}

	AstVariabilityExpr parse_variability() {
		auto specifier = VariabilitySpecifier::Var;

		std::vector<AstNode> arguments;
		if (cursor.match(TokenKind::LeftBrace)) {
			uint32_t saved = open_angles;
			open_angles = 0;
			Defer _ = [&] {
				open_angles = saved;
			};

			specifier = VariabilitySpecifier::VarBounded;
			if (!cursor.match(TokenKind::RightBrace)) {
				do {
					arguments.emplace_back(parse_subexpression());
				} while (cursor.match(TokenKind::Comma));

				if (cursor.match(TokenKind::Ellipsis)) {
					specifier = VariabilitySpecifier::VarUnbounded;
				}

				expect(TokenKind::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		return {specifier, ast.store_multiple(arguments)};
	}

	AstNode parse_type() {
		if (cursor.match(TokenKind::Caret)) {
			return parse_pointer_type();
		}
		if (cursor.match(TokenKind::LeftBracket)) {
			return parse_array_type();
		}
		if (cursor.match(TokenKind::LeftParen)) {
			return parse_function_type();
		}

		auto name = expect_name(Message::ExpectType);
		return parse_name(name);
	}

	AstNode parse_array_type() {
		OptionalAstId bound;
		if (!cursor.match(TokenKind::RightBracket)) {
			bound = ast.store(parse_subexpression());
			expect(TokenKind::RightBracket, Message::ExpectBracketAfterArrayBound);
		}

		auto type = ast.store(parse_type());
		return {AstArrayTypeExpr {bound, type}};
	}

	AstNode parse_pointer_type() {
		AstVariabilityExpr variability;
		if (cursor.match(TokenKind::Var)) {
			variability = parse_variability();
		}

		auto type = ast.store(parse_type());
		return {AstPointerTypeExpr {variability, type}};
	}

	AstNode parse_function_type() {
		auto parameters = parse_function_type_parameters();
		expect(TokenKind::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto outputs = parse_function_type_outputs();
		return {AstFunctionTypeExpr {std::move(parameters), std::move(outputs)}};
	}

	std::vector<AstFunctionTypeExpr::Parameter> parse_function_type_parameters() {
		std::vector<AstFunctionTypeExpr::Parameter> parameters;
		if (!cursor.match(TokenKind::RightParen)) {
			do {
				parameters.emplace_back(parse_function_type_parameter());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionTypeExpr::Parameter parse_function_type_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto type = ast.store(parse_type());

		std::string_view name;
		if (auto name_token = cursor.match_name()) {
			name = name_token->get_lexeme(source);
		}

		if (auto equal = cursor.match_token(TokenKind::Equals)) {
			auto location = equal->locate_in(source);
			report(Message::FuncTypeDefaultArgument, location);
			throw ParseError();
		}
		return {specifier, type, name};
	}

	std::vector<AstFunctionTypeExpr::Output> parse_function_type_outputs() {
		std::vector<AstFunctionTypeExpr::Output> outputs;
		if (cursor.match(TokenKind::LeftParen)) {
			do {
				outputs.emplace_back(parse_function_type_output());
			} while (cursor.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterOutputs);
		} else {
			auto type = ast.store(parse_type());
			outputs.emplace_back(type, std::string_view());
		}

		return outputs;
	}

	AstFunctionTypeExpr::Output parse_function_type_output() {
		auto type = ast.store(parse_type());

		std::string_view name;
		if (auto token = cursor.match_name()) {
			name = token->get_lexeme(source);
		}

		return {type, name};
	}

	void rescind_lookahead(size_t saved_node_count) {
		auto first = ast.ast_nodes.begin() + static_cast<ptrdiff_t>(saved_node_count);
		ast.ast_nodes.erase(first, ast.ast_nodes.end());
	}

	void expect(TokenKind kind, Message message) {
		auto token = cursor.peek();
		if (token.kind == kind) {
			cursor.advance();
		} else {
			report_expectation(message, token);
			throw ParseError();
		}
	}

	std::string_view expect_name(Message message) {
		auto token = cursor.peek();
		if (token.kind == TokenKind::Name) {
			cursor.advance();
			return token.get_lexeme(source);
		}

		report_expectation(message, token);
		return {};
	}

	void report_expectation(Message message, Token unexpected) {
		auto location = unexpected.locate_in(source);
		report(message, location, unexpected.to_message_string(source));
	}

	template<typename... Args>
	void report(Message message, SourceLocation location, Args&&... args) {
		if (!is_looking_ahead) {
			reporter.report(message, location, std::forward<Args>(args)...);
		}
	}

	static Precedence lookup_precedence_for_associativity(BinaryOperator op) {
		using enum BinaryOperator;
		switch (op) {
			case Add:
			case Subtract: return Precedence::Bitwise;
			case Multiply:
			case Divide:
			case Remainder:
			case Power: return Precedence::Multiplicative; // using a lower precedence for power here makes it right-associative
			case LogicalAnd:
			case LogicalOr: return Precedence::Logical;
			case BitAnd:
			case BitOr:
			case Xor:
			case LeftShift:
			case RightShift: return Precedence::Bitwise;
			case Equal:
			case NotEqual:
			case Less:
			case Greater:
			case LessEqual:
			case GreaterEqual: return Precedence::Comparison;
			case Assign:
			case AddAssign:
			case SubtractAssign:
			case MultiplyAssign:
			case DivideAssign:
			case RemainderAssign:
			case PowerAssign:
			case AndAssign:
			case OrAssign:
			case XorAssign:
			case LeftShiftAssign:
			case RightShiftAssign: return Precedence::Assignment;
		}
		fail_unreachable();
	}
};

Ast parse(const Source& source, Reporter& reporter) {
	auto token_stream = lex(source, reporter);
	return parse(token_stream, source, reporter);
}

Ast parse(const TokenStream& token_stream, const Source& source, Reporter& reporter) {
	Parser parser(token_stream, source, reporter);
	return parser.parse();
}

} // namespace cero
