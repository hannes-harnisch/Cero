#include "Parse.hpp"

#include "syntax/Lex.hpp"
#include "syntax/Literal.hpp"
#include "syntax/ParseCursor.hpp"
#include "util/Algorithm.hpp"
#include "util/Defer.hpp"
#include "util/Fail.hpp"

namespace cero {

class Parser {
	Ast			  ast;
	const Source& source;
	Reporter&	  reporter;
	ParseCursor	  cursor;
	bool		  should_report = true;
	uint32_t	  expr_depth	= 0;
	uint32_t	  open_angles	= 0;

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
				synchronize_at_definition_scope();
			}
		}
		auto defs = ast.store_multiple(definitions);
		ast.store(AstRoot {defs});
		return std::move(ast);
	}

private:
	enum class Precedence : uint8_t {
		Statement,
		Assignment,
		Logical,
		Comparison,
		Additive,
		Bitwise = Additive,
		Multiplicative,
		Prefix,
		Postfix
	};

	using PrefixParse	 = AstNode (Parser::*)();
	using NonPrefixParse = AstNode (Parser::*)(AstId);

	struct NonPrefixParseRule {
		Precedence	   precedence = Precedence::Statement;
		NonPrefixParse func		  = nullptr;
	};

	struct ParseError {};

	AstNode parse_definition() {
		auto access_specifier = AccessSpecifier::None;
		if (cursor.match(Token::Private))
			access_specifier = AccessSpecifier::Private;
		else if (cursor.match(Token::Public))
			access_specifier = AccessSpecifier::Public;

		if (cursor.match(Token::Name))
			return parse_function(access_specifier);

		if (cursor.match(Token::Struct))
			return parse_struct(access_specifier);

		if (cursor.match(Token::Enum))
			return parse_enum(access_specifier);

		report_expectation(Message::ExpectFuncStructEnum, cursor.peek());
		throw ParseError();
	}

	void synchronize_at_definition_scope() {
		static constexpr Token sync_points[] {Token::Public, Token::Private, Token::Struct, Token::Enum, Token::EndOfFile};

		Token kind;
		do {
			cursor.advance();
			kind = cursor.peek_kind();
		} while (!contains(sync_points, kind));
	}

	AstNode parse_struct(AccessSpecifier) {
		to_do();
	}

	AstNode parse_enum(AccessSpecifier) {
		to_do();
	}

	AstNode parse_function(AccessSpecifier access_specifier) {
		auto name = cursor.previous().get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs	= parse_function_definition_outputs();
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
			throw ParseError();

		OptionalAstId default_argument;
		if (cursor.match(Token::Equals))
			default_argument = ast.store(parse_expression());

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

		std::string_view name;
		if (auto token = cursor.match(Token::Name))
			name = token->get_lexeme(source);

		return {type, name};
	}

	AstIdSet parse_block() {
		uint32_t saved_expr_depth = expr_depth;
		uint32_t saved_angles	  = open_angles;

		expr_depth	= 0;
		open_angles = 0;

		std::vector<AstNode> statements;
		while (!cursor.match(Token::RightBrace)) {
			try {
				statements.emplace_back(parse_statement());
			} catch (ParseError) {
				bool at_end = synchronize_at_statement_scope();
				if (at_end)
					break;
			}
		}

		expr_depth	= saved_expr_depth;
		open_angles = saved_angles;
		return ast.store_multiple(statements);
	}

	bool synchronize_at_statement_scope() {
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
		auto expression = parse_expression();

		auto kind = cursor.previous().kind;
		if (kind != Token::RightBrace && kind != Token::Semicolon) {
			if (auto name = cursor.match(Token::Name)) {
				expression = on_trailing_name(std::move(expression), *name);
			}
		}

		terminate_statement();
		return expression;
	}

	void terminate_statement() {
		auto kind = cursor.previous().kind;
		if (kind != Token::RightBrace && kind != Token::Semicolon)
			expect(Token::Semicolon, Message::ExpectSemicolon);
	}

	AstNode on_trailing_name(AstNode left_expr, LexicalToken name_token) {
		auto type = left_expr.get_type();

		using enum AstNodeKind;
		if (type != IdentifierExpr && type != GenericIdentifierExpr && type != MemberExpr && type != ArrayTypeExpr
			&& type != PointerTypeExpr && type != FunctionTypeExpr) {
			report_expectation(Message::ExpectSemicolon, name_token);
			throw ParseError();
		}

		auto name = name_token.get_lexeme(source);
		auto left = ast.store(std::move(left_expr));

		OptionalAstId initializer;
		if (cursor.match(Token::Equals))
			initializer = ast.store(parse_expression());

		return {AstBindingStatement {BindingSpecifier::Let, left, name, initializer}};
	}

	AstNode parse_expression(Precedence precedence = Precedence::Statement) {
		auto next = cursor.next();

		auto parse_prefix = lookup_prefix_parse(next.kind);
		if (parse_prefix == nullptr) {
			report_expectation(Message::ExpectExpr, next);
			throw ParseError();
		}

		++expr_depth;
		defer {
			--expr_depth;
		};

		cursor.advance();
		auto expression = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence)) {
			expression = (this->*parse)(ast.store(std::move(expression)));
		}

		return expression;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence current_precedence) {
		auto token = cursor.next();

		if (token.kind == Token::RightAngle && open_angles != 0)
			return nullptr;

		auto [next_precedence, parse] = find_non_prefix_parse(token);
		if (current_precedence >= next_precedence)
			return nullptr;

		cursor.advance();
		return parse;
	}

	NonPrefixParseRule find_non_prefix_parse(LexicalToken token) {
		if (token.kind != Token::RightAngle)
			return lookup_non_prefix_parse(token.kind);

		auto saved = cursor;
		cursor.advance();
		auto next = cursor.next();
		if (next.kind == Token::RightAngle && next.offset == token.offset + 1)
			return {Precedence::Bitwise, &Parser::on_infix_operator<BinaryOperator::RightShift, Precedence::Bitwise>};

		cursor = saved;
		return {Precedence::Comparison, &Parser::on_infix_operator<BinaryOperator::Greater, Precedence::Comparison>};
	}

	AstNode on_if() {
		auto condition = ast.store(parse_expression());
		expect_colon_or_block();

		auto then_expr = ast.store(parse_expression());
		if (expr_depth == 1)
			terminate_statement();

		OptionalAstId else_expr;
		if (cursor.match(Token::Else))
			else_expr = ast.store(parse_expression());

		return {AstIfStatement {condition, then_expr, else_expr}};
	}

	AstNode on_while() {
		auto condition = ast.store(parse_expression());
		expect_colon_or_block();
		auto statement = ast.store(parse_expression());
		return {AstWhileLoop {condition, statement}};
	}

	AstNode on_for() {
		to_do();
	}

	AstNode on_left_brace() {
		return {AstBlockStatement {parse_block()}};
	}

	void expect_colon_or_block() {
		if (auto colon = cursor.match(Token::Colon)) {
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
			initializer = ast.store(parse_expression());

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
		if (auto name_token = cursor.match(Token::Name)) {
			if (cursor.match(Token::Equals)) {
				auto name		 = name_token->get_lexeme(source);
				auto initializer = ast.store(parse_expression());
				return {specifier, {}, name, initializer};
			}
			cursor = saved;
		}

		auto type = ast.store(parse_type());
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalAstId initializer;
		if (cursor.match(Token::Equals))
			initializer = ast.store(parse_expression());

		return {specifier, type, name, initializer};
	}

	AstNode on_name() {
		auto name = cursor.previous().get_lexeme(source);
		return parse_identifier(name);
	}

	AstNode parse_identifier(std::string_view name) {
		if (cursor.match(Token::LeftAngle))
			return parse_generic_identifier(name);

		return {AstIdentifierExpr {name}};
	}

	AstNode parse_generic_identifier(std::string_view name) {
		++open_angles;
		--expr_depth;
		defer {
			++expr_depth;
			--open_angles;
		};

		std::vector<AstNode> generic_args;
		if (!cursor.match(Token::RightAngle)) {
			auto   saved_cursor		= cursor;
			size_t saved_node_count = ast.ast_nodes.size();

			bool fall_back = should_fall_back_to_identifier();

			cursor = saved_cursor;
			rescind_lookahead(saved_node_count);

			if (fall_back) {
				cursor.retreat();
				return {AstIdentifierExpr {name}};
			}

			do
				generic_args.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			cursor.advance();
		}
		return {AstGenericIdentifierExpr {name, ast.store_multiple(generic_args)}};
	}

	bool should_fall_back_to_identifier() {
		bool saved	  = should_report;
		should_report = false;
		defer {
			should_report = saved;
		};

		do
			parse_expression();
		while (cursor.match(Token::Comma));

		static constexpr Token fallbacks[] {Token::Name,		  Token::DecIntLiteral, Token::HexIntLiteral,
											Token::BinIntLiteral, Token::OctIntLiteral, Token::FloatLiteral,
											Token::CharLiteral,	  Token::StringLiteral, Token::Minus,
											Token::Tilde,		  Token::Ampersand,		Token::Bang,
											Token::PlusPlus,	  Token::MinusMinus};
		if (cursor.match(Token::RightAngle)) {
			auto kind = cursor.next_kind();
			return (contains(fallbacks, kind) && expr_depth != 0) || (open_angles == 1 && kind == Token::RightAngle);
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
		open_angles	   = 0;
		defer {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(Token::RightParen)) {
			do
				arguments.emplace_back(parse_expression());
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
		open_angles	   = 0;
		defer {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(Token::RightBracket)) {
			do
				arguments.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			expect(Token::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return ast.store_multiple(arguments);
	}

	AstNode on_break() {
		return {AstBreakExpr {parse_optional_expression()}};
	}

	AstNode on_continue() {
		return {AstContinueExpr {parse_optional_expression()}};
	}

	AstNode on_return() {
		return {AstReturnExpr {parse_optional_expression()}};
	}

	AstNode on_throw() {
		return {AstThrowExpr {parse_optional_expression()}};
	}

	OptionalAstId parse_optional_expression() {
		auto next = cursor.next_kind();
		if (lookup_prefix_parse(next) == nullptr)
			return {};

		return ast.store(parse_expression());
	}

	template<UnaryOperator O>
	AstNode on_prefix_operator() {
		auto right = ast.store(parse_expression(Precedence::Prefix));
		return {AstUnaryExpr {O, right}};
	}

	template<BinaryOperator O, Precedence P>
	AstNode on_infix_operator(AstId left) {
		auto target = cursor.previous();
		auto right	= ast.store(parse_expression(P));
		validate_associativity(O, left, right, target);
		return {AstBinaryExpr {O, left, right}};
	}

	template<UnaryOperator O>
	AstNode on_postfix_operator(AstId left) {
		return {AstUnaryExpr {O, left}};
	}

	void validate_associativity(BinaryOperator current, AstId left_id, AstId right_id, LexicalToken target) {
		auto& left_node	 = ast.get(left_id);
		auto& right_node = ast.get(right_id);

		if (auto right = right_node.get<AstBinaryExpr>())
			validate_binary_associativity(current, right->op, target);

		if (auto left = left_node.get<AstBinaryExpr>())
			validate_binary_associativity(left->op, current, target);
		else if (auto unary = left_node.get<AstUnaryExpr>())
			validate_unary_binary_associativity(unary->op, current, target);
	}

	void validate_binary_associativity(BinaryOperator left, BinaryOperator right, LexicalToken target) {
		bool chains = false;
		if (associates_arithmetic_and_bitwise(left, right) || associates_different_logical_operators(left, right)
			|| associates_comparison_operators(left, right, chains)) {
			auto location = target.locate_in(source);
			if (chains)
				report(Message::AmbiguousOperatorChaining, location, to_string(left));
			else
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

	static bool associates_comparison_operators(BinaryOperator left, BinaryOperator right, bool& chains) {
		static constexpr BinaryOperator comparison_operators[] {BinaryOperator::Equal,	   BinaryOperator::NotEqual,
																BinaryOperator::Less,	   BinaryOperator::Greater,
																BinaryOperator::LessEqual, BinaryOperator::GreaterEqual};

		chains = left == right;
		return contains(comparison_operators, left) && contains(comparison_operators, right);
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
		open_angles	   = 0;
		defer {
			open_angles = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor.match(Token::RightParen)) {
			do
				arguments.emplace_back(parse_expression());
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

	AstVariabilityExpr parse_variability() {
		auto specifier = VariabilitySpecifier::Var;

		std::vector<AstNode> arguments;
		if (cursor.match(Token::LeftBrace)) {
			uint32_t saved = open_angles;
			open_angles	   = 0;
			defer {
				open_angles = saved;
			};

			specifier = VariabilitySpecifier::VarBounded;
			if (!cursor.match(Token::RightBrace)) {
				do
					arguments.emplace_back(parse_expression());
				while (cursor.match(Token::Comma));

				if (cursor.match(Token::Ellipsis))
					specifier = VariabilitySpecifier::VarUnbounded;

				expect(Token::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		return {specifier, ast.store_multiple(arguments)};
	}

	AstNode parse_type() {
		const auto saved = expr_depth;
		expr_depth		 = 1;
		defer {
			expr_depth = saved;
		};

		if (cursor.match(Token::Caret))
			return parse_pointer_type();
		if (cursor.match(Token::LeftBracket))
			return parse_array_type();
		if (cursor.match(Token::LeftParen))
			return parse_function_type();

		auto name = expect_name(Message::ExpectType);
		return parse_identifier(name);
	}

	AstNode parse_array_type() {
		OptionalAstId bound;
		if (!cursor.match(Token::RightBracket)) {
			bound = ast.store(parse_expression());
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
		if (auto name_token = cursor.match(Token::Name))
			name = name_token->get_lexeme(source);

		if (auto equal = cursor.match(Token::Equals)) {
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
			outputs.emplace_back(AstFunctionTypeExpr::Output {type, std::string_view()});
		}

		return outputs;
	}

	AstFunctionTypeExpr::Output parse_function_type_output() {
		auto type = ast.store(parse_type());

		std::string_view name;
		if (auto token = cursor.match(Token::Name))
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
		if (should_report)
			reporter.report(message, location, std::forward<Args>(args)...);
	}

	static PrefixParse lookup_prefix_parse(Token kind) {
		using enum Token;
		using enum UnaryOperator;

		switch (kind) {
			case Name: return &Parser::on_name;
			case If: return &Parser::on_if;
			case While: return &Parser::on_while;
			case For: return &Parser::on_for;
			case LeftBrace: return &Parser::on_left_brace;
			case Let: return &Parser::on_let;
			case Var: return &Parser::on_var;
			case Static: return &Parser::on_static;
			case Const: return &Parser::on_const;
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

	static NonPrefixParseRule lookup_non_prefix_parse(Token kind) {
		using enum Token;
		using enum Precedence;
		using enum UnaryOperator;
		using enum BinaryOperator;

		switch (kind) {
			case Equals: return {Assignment, &Parser::on_infix_operator<Assign, Assignment>};
			case PlusEquals: return {Assignment, &Parser::on_infix_operator<AddAssign, Assignment>};
			case MinusEquals: return {Assignment, &Parser::on_infix_operator<SubtractAssign, Assignment>};
			case StarEquals: return {Assignment, &Parser::on_infix_operator<MultiplyAssign, Assignment>};
			case SlashEquals: return {Assignment, &Parser::on_infix_operator<DivideAssign, Assignment>};
			case PercentEquals: return {Assignment, &Parser::on_infix_operator<RemainderAssign, Assignment>};
			case StarStarEquals: return {Assignment, &Parser::on_infix_operator<PowerAssign, Assignment>};
			case AmpersandEquals: return {Assignment, &Parser::on_infix_operator<AndAssign, Assignment>};
			case PipeEquals: return {Assignment, &Parser::on_infix_operator<OrAssign, Assignment>};
			case TildeEquals: return {Assignment, &Parser::on_infix_operator<XorAssign, Assignment>};
			case LeftAngleAngleEquals: return {Assignment, &Parser::on_infix_operator<LeftShiftAssign, Assignment>};
			case RightAngleAngleEquals: return {Assignment, &Parser::on_infix_operator<RightShiftAssign, Assignment>};
			case AmpersandAmpersand: return {Logical, &Parser::on_infix_operator<LogicalAnd, Logical>};
			case PipePipe: return {Logical, &Parser::on_infix_operator<LogicalOr, Logical>};
			case EqualsEquals: return {Comparison, &Parser::on_infix_operator<Equal, Comparison>};
			case BangEquals: return {Comparison, &Parser::on_infix_operator<NotEqual, Comparison>};
			case LeftAngle: return {Comparison, &Parser::on_infix_operator<Less, Comparison>};
			case LeftAngleEquals: return {Comparison, &Parser::on_infix_operator<LessEqual, Comparison>};
			case RightAngleEquals: return {Comparison, &Parser::on_infix_operator<GreaterEqual, Comparison>};
			case Plus: return {Additive, &Parser::on_infix_operator<Add, Additive>};
			case Minus: return {Additive, &Parser::on_infix_operator<Subtract, Additive>};
			case Ampersand: return {Bitwise, &Parser::on_infix_operator<BitAnd, Bitwise>};
			case Pipe: return {Bitwise, &Parser::on_infix_operator<BitOr, Bitwise>};
			case Tilde: return {Bitwise, &Parser::on_infix_operator<Xor, Bitwise>};
			case LeftAngleAngle: return {Bitwise, &Parser::on_infix_operator<LeftShift, Bitwise>};
			case Star: return {Multiplicative, &Parser::on_infix_operator<Multiply, Multiplicative>};
			case Slash: return {Multiplicative, &Parser::on_infix_operator<Divide, Multiplicative>};
			case Percent: return {Multiplicative, &Parser::on_infix_operator<Remainder, Multiplicative>};
			case StarStar: return {Prefix, &Parser::on_infix_operator<Power, Multiplicative>}; // lower for right-associativity
			case Caret: return {Postfix, &Parser::on_postfix_operator<Dereference>};
			case PlusPlus: return {Postfix, &Parser::on_postfix_operator<PostIncrement>};
			case MinusMinus: return {Postfix, &Parser::on_postfix_operator<PostDecrement>};
			case Dot: return {Postfix, &Parser::on_dot};
			case LeftParen: return {Postfix, &Parser::on_infix_left_paren};
			case LeftBracket: return {Postfix, &Parser::on_infix_left_bracket};
			default: return {};
		}
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
