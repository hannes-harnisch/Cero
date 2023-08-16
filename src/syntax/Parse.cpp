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
		while (!cursor.match(Token::EndOfFile)) {
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
		if (cursor.match(Token::Private))
			access_specifier = AccessSpecifier::Private;
		else if (cursor.match(Token::Public))
			access_specifier = AccessSpecifier::Public;

		if (auto name_token = cursor.match_name())
			return parse_function(access_specifier, *name_token);

		if (cursor.match(Token::Struct))
			return parse_struct(access_specifier);

		if (cursor.match(Token::Enum))
			return parse_enum(access_specifier);

		report_expectation(Message::ExpectFuncStructEnum, cursor.peek());
		throw ParseError();
	}

	void recover_at_definition_scope() {
		static constexpr Token recovery_tokens[] {Token::Public, Token::Private, Token::Struct, Token::Enum, Token::EndOfFile};

		Token kind;
		do {
			cursor.advance();
			kind = cursor.peek_kind();
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

	AstNode parse_function(AccessSpecifier access_specifier, LexicalToken name_token) {
		auto name = name_token.get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs = parse_function_definition_outputs();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

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
		if (!cursor.match(Token::RightParen)) {
			do
				parameters.emplace_back(parse_function_definition_parameter());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionDefinition::Parameter parse_function_definition_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor.match(Token::In))
			specifier = ParameterSpecifier::In;
		else if (cursor.match(Token::Var))
			specifier = ParameterSpecifier::Var;

		auto type = ast.store(parse_type());
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty())
			throw ParseError(); // TODO: explain why this is necessary

		OptionalAstId default_argument;
		if (cursor.match(Token::Equals))
			default_argument = ast.store(parse_subexpression());

		return {specifier, type, name, default_argument};
	}

	std::vector<AstFunctionDefinition::Output> parse_function_definition_outputs() {
		std::vector<AstFunctionDefinition::Output> outputs;
		if (cursor.match(Token::ThinArrow)) {
			do
				outputs.emplace_back(parse_function_definition_output());
			while (cursor.match(Token::Comma));
		}
		return outputs;
	}

	AstFunctionDefinition::Output parse_function_definition_output() {
		auto type = ast.store(parse_type());

		StringId name;
		if (auto name_token = cursor.match_name())
			name = name_token->get_lexeme(source);

		return {type, name};
	}

	AstIdSet parse_block() {
		const uint32_t saved_angles = open_angles;
		const bool saved_binding_allowed = is_binding_allowed;
		open_angles = 0;
		is_binding_allowed = true;

		std::vector<AstNode> statements;
		while (!cursor.match(Token::RightBrace)) {
			try {
				statements.emplace_back(parse_statement());
			} catch (ParseError) {
				bool at_end = recover_at_statement_scope();
				if (at_end)
					break;
			}
		}

		open_angles = saved_angles;
		is_binding_allowed = saved_binding_allowed;
		return ast.store_multiple(statements);
	}

	bool recover_at_statement_scope() {
		Token kind = cursor.peek_kind();
		while (kind != Token::EndOfFile) {
			if (kind == Token::Semicolon) {
				cursor.advance();
				return false;
			}
			if (kind == Token::RightBrace)
				return false;

			cursor.advance();
			kind = cursor.peek_kind();
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
			expect(Token::Semicolon, Message::ExpectSemicolon);
		}
		return stmt;
	}

	PrefixParse lookup_statement_parse(bool& parses_complete_stmt) {
		PrefixParse parse;

		auto next = cursor.next_kind();
		using enum Token;
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

	AstNode on_trailing_name(AstNode left_expr, LexicalToken name_token) {
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
		if (cursor.match(Token::Equals))
			initializer = ast.store(parse_subexpression());

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
		auto next = cursor.next();

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

	static PrefixParse lookup_prefix_parse(Token kind) {
		using enum Token;
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
		auto token = cursor.next();

		if (token.kind == Token::RightAngle && open_angles != 0)
			return nullptr;

		auto rule = find_non_prefix_parse(token);
		if (current_precedence >= rule.precedence)
			return nullptr;

		cursor.advance();
		return rule.func;
	}

	NonPrefixParseRule find_non_prefix_parse(LexicalToken token) {
		if (token.kind != Token::RightAngle)
			return lookup_non_prefix_parse(token.kind);

		auto saved = cursor;
		cursor.advance();
		auto next = cursor.next();
		if (next.kind == Token::RightAngle && next.offset == token.offset + 1)
			return {Precedence::Bitwise, &Parser::on_infix_operator<BinaryOperator::RightShift>};

		cursor = saved;
		return {Precedence::Comparison, &Parser::on_infix_operator<BinaryOperator::Greater>};
	}

	AstNode on_if_stmt() {
		auto condition = ast.store(parse_expression_or_binding());
		expect_colon_or_block();

		auto then_stmt = ast.store(parse_statement());

		OptionalAstId else_stmt;
		if (cursor.match(Token::Else))
			else_stmt = ast.store(parse_statement());

		return {AstIfExpr {condition, then_stmt, else_stmt}};
	}

	AstNode on_if_expr() {
		auto condition = ast.store(parse_expression_or_binding());
		expect(Token::Colon, Message::ExpectColonInIfExpr);
		auto then_expr = ast.store(parse_subexpression());
		expect(Token::Else, Message::ExpectElse);
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
		if (auto colon = cursor.match_token(Token::Colon)) {
			auto next = cursor.next();
			if (next.kind == Token::LeftBrace)
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source));
		} else {
			auto next = cursor.next();
			if (next.kind != Token::LeftBrace)
				report_expectation(Message::ExpectColonOrBlock, next);
		}
	}

	AstNode on_let() {
		auto name = expect_name(Message::ExpectNameAfterLet);

		OptionalAstId initializer;
		if (cursor.match(Token::Equals))
			initializer = ast.store(parse_subexpression());

		return {AstBindingStatement {BindingSpecifier::Let, {}, name, initializer}};
	}

	AstNode on_var() {
		if (cursor.next_kind() == Token::LeftBrace)
			return {parse_variability()};

		return {parse_binding(BindingSpecifier::Var)};
	}

	AstNode on_const() {
		return {parse_binding(BindingSpecifier::Const)};
	}

	AstNode on_static() {
		auto specifier = BindingSpecifier::Static;
		if (cursor.match(Token::Var))
			specifier = BindingSpecifier::StaticVar;

		return {parse_binding(specifier)};
	}

	AstBindingStatement parse_binding(BindingSpecifier specifier) {
		auto saved = cursor;
		if (auto name_token = cursor.match_name()) {
			if (cursor.match(Token::Equals)) {
				auto name = name_token->get_lexeme(source);
				auto initializer = ast.store(parse_subexpression());
				return {specifier, {}, name, initializer};
			}
			cursor = saved;
		}

		auto type = ast.store(parse_type());
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalAstId initializer;
		if (cursor.match(Token::Equals))
			initializer = ast.store(parse_subexpression());

		return {specifier, type, name, initializer};
	}

	AstNode on_name() {
		auto name = cursor.previous().get_lexeme(source);
		return parse_name(name);
	}

	AstNode parse_name(std::string_view name) {
		auto saved_cursor = cursor;
		if (cursor.match(Token::LeftAngle))
			return parse_generic_name(name, saved_cursor);

		return {AstNameExpr {name}};
	}

	AstNode parse_generic_name(std::string_view name, ParseCursor name_start) {
		++open_angles;
		Defer _ = [&] {
			--open_angles;
		};

		std::vector<AstNode> generic_args;
		if (!cursor.match(Token::RightAngle)) {
			size_t saved_node_count = ast.ast_nodes.size();

			bool fall_back = should_fall_back_to_name();

			cursor = name_start;
			rescind_lookahead(saved_node_count);

			if (fall_back)
				return {AstNameExpr {name}};

			cursor.advance();
			do
				generic_args.emplace_back(parse_subexpression());
			while (cursor.match(Token::Comma));
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

		do
			parse_subexpression();
		while (cursor.match(Token::Comma));

		static constexpr Token fallbacks[] {Token::DecIntLiteral, Token::HexIntLiteral, Token::BinIntLiteral,
											Token::OctIntLiteral, Token::FloatLiteral,	Token::CharLiteral,
											Token::StringLiteral, Token::Minus,			Token::Tilde,
											Token::Ampersand,	  Token::Bang,			Token::PlusPlus,
											Token::MinusMinus};
		if (cursor.match(Token::RightAngle)) {
			auto kind = cursor.next_kind();
			return (kind == Token::Name && !is_binding_allowed) || contains(fallbacks, kind)
				   || (open_angles == 1 && kind == Token::RightAngle);
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
		if (!cursor.match(Token::RightParen)) {
			do
				arguments.emplace_back(parse_subexpression());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectClosingParen);
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
		if (!cursor.match(Token::RightBracket)) {
			do
				arguments.emplace_back(parse_subexpression());
			while (cursor.match(Token::Comma));
			expect(Token::RightBracket, Message::ExpectBracketAfterIndex);
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
			do
				values.emplace_back(parse_subexpression());
			while (cursor.match(Token::Comma));
		}

		auto return_values = ast.store_multiple(values);
		return {AstReturnExpr {return_values}};
	}

	AstNode on_throw() {
		auto expr = parse_optional_subexpression();
		return {AstThrowExpr {expr}};
	}

	OptionalAstId parse_optional_subexpression() {
		if (expression_may_follow())
			return ast.store(parse_subexpression());

		return {};
	}

	bool expression_may_follow() {
		auto next = cursor.next_kind();
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

	void validate_associativity(BinaryOperator current, AstId left_id, AstId right_id, LexicalToken target) {
		auto& left_node = ast.get(left_id);
		auto& right_node = ast.get(right_id);

		if (auto right = right_node.get<AstBinaryExpr>())
			validate_binary_associativity(current, right->op, target);

		if (auto left = left_node.get<AstBinaryExpr>())
			validate_binary_associativity(left->op, current, target);
		else if (auto unary = left_node.get<AstUnaryExpr>())
			validate_unary_binary_associativity(unary->op, current, target);
	}

	void validate_binary_associativity(BinaryOperator left, BinaryOperator right, LexicalToken target) {
		if (associates_arithmetic_and_bitwise(left, right) || associates_different_logical_operators(left, right)
			|| associates_intransitive_comparison_operators(left, right)) {
			auto location = target.locate_in(source);
			report(Message::AmbiguousOperatorMixing, location, to_string(left), to_string(right));
		}
	}

	static bool associates_arithmetic_and_bitwise(BinaryOperator left, BinaryOperator right) {
		static constexpr BinaryOperator bitwise_operators[] {BinaryOperator::BitAnd, BinaryOperator::BitOr, BinaryOperator::Xor,
															 BinaryOperator::LeftShift, BinaryOperator::RightShift};

		static constexpr BinaryOperator arithmetic_operators[] {BinaryOperator::Add,	   BinaryOperator::Subtract,
																BinaryOperator::Multiply,  BinaryOperator::Divide,
																BinaryOperator::Remainder, BinaryOperator::Power};

		if (contains(bitwise_operators, left))
			return contains(arithmetic_operators, right);
		else
			return contains(arithmetic_operators, left) && contains(bitwise_operators, right);
	}

	static bool associates_different_logical_operators(BinaryOperator left, BinaryOperator right) {
		if (left == BinaryOperator::LogicalAnd)
			return right == BinaryOperator::LogicalOr;
		else
			return left == BinaryOperator::LogicalOr && right == BinaryOperator::LogicalAnd;
	}

	static bool associates_intransitive_comparison_operators(BinaryOperator left, BinaryOperator right) {
		static constexpr BinaryOperator comparison_operators[] {BinaryOperator::Equal,	   BinaryOperator::NotEqual,
																BinaryOperator::Less,	   BinaryOperator::Greater,
																BinaryOperator::LessEqual, BinaryOperator::GreaterEqual};

		struct OperatorPair {
			BinaryOperator left, right;

			bool operator==(const OperatorPair&) const = default;
		};

		static constexpr OperatorPair allowed[] {{BinaryOperator::Equal, BinaryOperator::Equal},
												 {BinaryOperator::Less, BinaryOperator::Less},
												 {BinaryOperator::Less, BinaryOperator::LessEqual},
												 {BinaryOperator::LessEqual, BinaryOperator::LessEqual},
												 {BinaryOperator::LessEqual, BinaryOperator::Less},
												 {BinaryOperator::Greater, BinaryOperator::Greater},
												 {BinaryOperator::Greater, BinaryOperator::GreaterEqual},
												 {BinaryOperator::GreaterEqual, BinaryOperator::GreaterEqual},
												 {BinaryOperator::GreaterEqual, BinaryOperator::Greater}};

		return contains(comparison_operators, left) && contains(comparison_operators, right)
			   && !contains(allowed, OperatorPair {left, right});
	}

	void validate_unary_binary_associativity(UnaryOperator left, BinaryOperator right, LexicalToken target) {
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
		if (!cursor.match(Token::RightParen)) {
			do
				arguments.emplace_back(parse_subexpression());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectClosingParen);
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
		if (cursor.match(Token::LeftBrace)) {
			uint32_t saved = open_angles;
			open_angles = 0;
			Defer _ = [&] {
				open_angles = saved;
			};

			specifier = VariabilitySpecifier::VarBounded;
			if (!cursor.match(Token::RightBrace)) {
				do
					arguments.emplace_back(parse_subexpression());
				while (cursor.match(Token::Comma));

				if (cursor.match(Token::Ellipsis))
					specifier = VariabilitySpecifier::VarUnbounded;

				expect(Token::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		return {specifier, ast.store_multiple(arguments)};
	}

	AstNode parse_type() {
		if (cursor.match(Token::Caret))
			return parse_pointer_type();
		if (cursor.match(Token::LeftBracket))
			return parse_array_type();
		if (cursor.match(Token::LeftParen))
			return parse_function_type();

		auto name = expect_name(Message::ExpectType);
		return parse_name(name);
	}

	AstNode parse_array_type() {
		OptionalAstId bound;
		if (!cursor.match(Token::RightBracket)) {
			bound = ast.store(parse_subexpression());
			expect(Token::RightBracket, Message::ExpectBracketAfterArrayBound);
		}

		auto type = ast.store(parse_type());
		return {AstArrayTypeExpr {bound, type}};
	}

	AstNode parse_pointer_type() {
		AstVariabilityExpr variability;
		if (cursor.match(Token::Var))
			variability = parse_variability();

		auto type = ast.store(parse_type());
		return {AstPointerTypeExpr {variability, type}};
	}

	AstNode parse_function_type() {
		auto parameters = parse_function_type_parameters();
		expect(Token::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto outputs = parse_function_type_outputs();
		return {AstFunctionTypeExpr {std::move(parameters), std::move(outputs)}};
	}

	std::vector<AstFunctionTypeExpr::Parameter> parse_function_type_parameters() {
		std::vector<AstFunctionTypeExpr::Parameter> parameters;
		if (!cursor.match(Token::RightParen)) {
			do
				parameters.emplace_back(parse_function_type_parameter());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionTypeExpr::Parameter parse_function_type_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor.match(Token::In))
			specifier = ParameterSpecifier::In;
		else if (cursor.match(Token::Var))
			specifier = ParameterSpecifier::Var;

		auto type = ast.store(parse_type());

		std::string_view name;
		if (auto name_token = cursor.match_name())
			name = name_token->get_lexeme(source);

		if (auto equal = cursor.match_token(Token::Equals)) {
			auto location = equal->locate_in(source);
			report(Message::FuncTypeDefaultArgument, location);
			throw ParseError();
		}
		return {specifier, type, name};
	}

	std::vector<AstFunctionTypeExpr::Output> parse_function_type_outputs() {
		std::vector<AstFunctionTypeExpr::Output> outputs;
		if (cursor.match(Token::LeftParen)) {
			do
				outputs.emplace_back(parse_function_type_output());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterOutputs);
		} else {
			auto type = ast.store(parse_type());
			outputs.emplace_back(type, std::string_view());
		}

		return outputs;
	}

	AstFunctionTypeExpr::Output parse_function_type_output() {
		auto type = ast.store(parse_type());

		std::string_view name;
		if (auto token = cursor.match_name())
			name = token->get_lexeme(source);

		return {type, name};
	}

	void expect(Token kind, Message message) {
		auto token = cursor.next();
		if (token.kind == kind)
			cursor.advance();
		else {
			report_expectation(message, token);
			throw ParseError();
		}
	}

	std::string_view expect_name(Message message) {
		auto token = cursor.next();
		if (token.kind == Token::Name) {
			cursor.advance();
			return token.get_lexeme(source);
		}

		report_expectation(message, token);
		return {};
	}

	void rescind_lookahead(size_t saved_node_count) {
		auto first = ast.ast_nodes.begin() + static_cast<ptrdiff_t>(saved_node_count);
		ast.ast_nodes.erase(first, ast.ast_nodes.end());
	}

	void report_expectation(Message message, LexicalToken unexpected) {
		auto location = unexpected.locate_in(source);
		report(message, location, unexpected.to_message_string(source));
	}

	template<typename... Args>
	void report(Message message, SourceLocation location, Args&&... args) {
		if (!is_looking_ahead)
			reporter.report(message, location, std::forward<Args>(args)...);
	}

	static NonPrefixParseRule lookup_non_prefix_parse(Token kind) {
		using enum Token;
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

	static Precedence lookup_precedence_for_associativity(BinaryOperator op) {
		using enum BinaryOperator;
		using enum Precedence;

		switch (op) {
			case Add:
			case Subtract: return Bitwise;
			case Multiply:
			case Divide:
			case Remainder:
			case Power: return Multiplicative; // lower for right-associativity
			case LogicalAnd:
			case LogicalOr: return Logical;
			case BitAnd:
			case BitOr:
			case Xor:
			case LeftShift:
			case RightShift: return Bitwise;
			case Equal:
			case NotEqual:
			case Less:
			case Greater:
			case LessEqual:
			case GreaterEqual: return Comparison;
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
			case RightShiftAssign: return Assignment;
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
