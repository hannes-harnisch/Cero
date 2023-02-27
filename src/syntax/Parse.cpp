#include "cero/syntax/Parse.hpp"

#include "cero/syntax/AstBuilder.hpp"
#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Literal.hpp"
#include "cero/util/LookupTable.hpp"
#include "syntax/AstString.hpp"
#include "syntax/ParseCursor.hpp"
#include "util/Algorithm.hpp"
#include "util/Defer.hpp"
#include "util/Fail.hpp"

namespace cero
{

class Parser
{
	AstBuilder	  ast;
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
		cursor(token_stream.begin())
	{}

	SyntaxTree parse()
	{
		while (!cursor.match(Token::EndOfFile))
		{
			try
			{
				ast.add_to_root(parse_definition());
			}
			catch (ParseError)
			{
				synchronize_definition();
			}
		}
		return SyntaxTree(std::move(ast));
	}

private:
	enum class Precedence : uint8_t
	{
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

	using PrefixParse	 = ExpressionNode (Parser::*)();
	using NonPrefixParse = ExpressionNode (Parser::*)(Expression);

	struct NonPrefixParseRule
	{
		Precedence	   precedence = Precedence::Statement;
		NonPrefixParse parse_func = nullptr;
	};

	struct ParseError
	{};

	Definition parse_definition()
	{
		auto access_specifier = ast::AccessSpecifier::None;
		if (cursor.match(Token::Private))
			access_specifier = ast::AccessSpecifier::Private;
		else if (cursor.match(Token::Public))
			access_specifier = ast::AccessSpecifier::Public;

		if (cursor.match(Token::Name))
			return parse_function(access_specifier);

		if (cursor.match(Token::Struct))
			return parse_struct(access_specifier);

		if (cursor.match(Token::Enum))
			return parse_enum(access_specifier);

		report_expectation(Message::ExpectFuncStructEnum, cursor.peek());
		throw ParseError();
	}

	void synchronize_definition()
	{
		static constexpr Token sync_points[] {Token::Public, Token::Private, Token::Struct, Token::Enum, Token::EndOfFile};

		Token kind;
		do
		{
			cursor.advance();
			kind = cursor.peek_kind();
		}
		while (!contains(sync_points, kind));
	}

	Definition parse_struct(ast::AccessSpecifier)
	{
		to_do();
	}

	Definition parse_enum(ast::AccessSpecifier)
	{
		to_do();
	}

	Definition parse_function(ast::AccessSpecifier access_specifier)
	{
		auto name = cursor.previous().get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs	= parse_function_outputs();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return ast.store(ast::Function {
			access_specifier,
			name,
			std::move(parameters),
			std::move(outputs),
			std::move(statements),
		});
	}

	std::vector<ast::Function::Parameter> parse_function_definition_parameters()
	{
		std::vector<ast::Function::Parameter> parameters;
		if (!cursor.match(Token::RightParen))
		{
			do
				parameters.emplace_back(parse_function_definition_parameter());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	ast::Function::Parameter parse_function_definition_parameter()
	{
		auto specifier = ast::ParameterSpecifier::In;
		if (cursor.match(Token::Let))
			specifier = ast::ParameterSpecifier::Let;
		else if (cursor.match(Token::Var))
			specifier = ast::ParameterSpecifier::Var;

		auto type = parse_type();
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty())
			throw ParseError();

		OptionalExpression default_argument;
		if (cursor.match(Token::Equals))
			default_argument = OptionalExpression(parse_expression());

		return {specifier, name, type, default_argument};
	}

	std::vector<ast::FunctionOutput> parse_function_outputs()
	{
		std::vector<ast::FunctionOutput> outputs;
		if (cursor.match(Token::ThinArrow))
		{
			do
				outputs.emplace_back(parse_function_output());
			while (cursor.match(Token::Comma));
		}
		return outputs;
	}

	ast::FunctionOutput parse_function_output()
	{
		auto type = parse_type();

		std::string_view name;
		if (auto token = cursor.match(Token::Name))
			name = token->get_lexeme(source);

		return {type, name};
	}

	std::vector<Expression> parse_block()
	{
		uint32_t saved_expr_depth = expr_depth;
		uint32_t saved_angles	  = open_angles;
		expr_depth				  = 0;
		open_angles				  = 0;

		std::vector<Expression> statements;
		while (!cursor.match(Token::RightBrace))
		{
			try
			{
				statements.emplace_back(parse_expression());
				terminate_statement();
			}
			catch (ParseError)
			{
				synchronize_statement();
			}
		}

		expr_depth	= saved_expr_depth;
		open_angles = saved_angles;
		return statements;
	}

	void terminate_statement()
	{
		auto kind = cursor.previous().kind;
		if (kind != Token::RightBrace && kind != Token::Semicolon)
			expect(Token::Semicolon, Message::ExpectSemicolon);
	}

	void synchronize_statement()
	{
		auto kind = cursor.peek_kind();
		while (kind != Token::Semicolon && kind != Token::RightBrace && kind != Token::EndOfFile)
		{
			cursor.advance();
			kind = cursor.peek_kind();
		}
	}

	Expression parse_expression(Precedence precedence = {})
	{
		auto next = cursor.next();

		auto parse_prefix = PREFIX_PARSES[next.kind];
		if (parse_prefix == nullptr)
		{
			report_expectation(Message::ExpectExpr, next);
			throw ParseError();
		}

		++expr_depth;
		defer
		{
			--expr_depth;
		};

		cursor.advance();
		auto expression = ast.store((this->*parse_prefix)());
		while (auto parse = get_next_non_prefix_parse(precedence))
			expression = ast.store((this->*parse)(expression));

		return expression;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence current_precedence)
	{
		auto token = cursor.next();

		if (token.kind == Token::RightAngle && open_angles != 0)
			return nullptr;

		auto [next_precedence, parse] = find_non_prefix_parse(token);
		if (current_precedence >= next_precedence)
			return nullptr;

		cursor.advance();
		return parse;
	}

	NonPrefixParseRule find_non_prefix_parse(LexicalToken token)
	{
		if (token.kind != Token::RightAngle)
			return NON_PREFIX_PARSES[token.kind];

		auto saved = cursor;
		cursor.advance();
		auto next = cursor.next();
		if (next.kind == Token::RightAngle && next.offset == token.offset + 1)
			return {Precedence::Bitwise, &Parser::on_infix_operator<ast::BinaryOperator::RightShift, Precedence::Bitwise>};

		cursor = saved;
		return {Precedence::Comparison, &Parser::on_infix_operator<ast::BinaryOperator::Greater, Precedence::Comparison>};
	}

	ExpressionNode on_if()
	{
		auto condition = parse_expression();
		expect_colon_or_block();

		auto then_expr = parse_expression();
		if (expr_depth == 1)
			terminate_statement();

		OptionalExpression else_expr;
		if (cursor.match(Token::Else))
			else_expr = parse_expression();

		return ast::If {condition, then_expr, else_expr};
	}

	ExpressionNode on_while()
	{
		auto condition = parse_expression();
		expect_colon_or_block();
		auto statement = parse_expression();
		return ast::WhileLoop {condition, statement};
	}

	ExpressionNode on_for()
	{
		to_do();
	}

	ExpressionNode on_left_brace()
	{
		return ast::Block {parse_block()};
	}

	void expect_colon_or_block()
	{
		if (auto colon = cursor.match(Token::Colon))
		{
			auto next = cursor.next();
			if (next.kind == Token::LeftBrace)
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source));
		}
		else
		{
			auto next = cursor.next();
			if (next.kind != Token::LeftBrace)
				report_expectation(Message::ExpectColonOrBlock, next);
		}
	}

	ExpressionNode on_let()
	{
		return parse_binding(ast::Binding::Specifier::Let);
	}

	ExpressionNode on_var()
	{
		if (cursor.next_kind() == Token::LeftBrace)
			return parse_variability();

		return parse_binding(ast::Binding::Specifier::Var);
	}

	ExpressionNode on_const()
	{
		return parse_binding(ast::Binding::Specifier::Const);
	}

	ExpressionNode on_static()
	{
		auto specifier = ast::Binding::Specifier::Static;
		if (cursor.match(Token::Var))
			specifier = ast::Binding::Specifier::StaticVar;

		return parse_binding(specifier);
	}

	ast::Binding parse_binding(ast::Binding::Specifier specifier)
	{
		auto saved = cursor;
		if (auto name_token = cursor.match(Token::Name))
		{
			if (cursor.match(Token::Equals))
			{
				auto name		 = name_token->get_lexeme(source);
				auto initializer = parse_expression();
				return {specifier, name, {}, initializer};
			}
			cursor = saved;
		}

		auto type = parse_type();
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalExpression initializer;
		if (cursor.match(Token::Equals))
			initializer = parse_expression();

		return {specifier, name, type, initializer};
	}

	ExpressionNode on_name()
	{
		auto name = cursor.previous().get_lexeme(source);
		return parse_identifier(name);
	}

	ExpressionNode parse_identifier(std::string_view name)
	{
		if (cursor.match(Token::LeftAngle))
			return parse_generic_identifier(name);

		return ast::Identifier {name};
	}

	ExpressionNode parse_generic_identifier(std::string_view name)
	{
		++open_angles;
		--expr_depth;
		defer
		{
			++expr_depth;
			--open_angles;
		};

		std::vector<Expression> generic_args;
		if (!cursor.match(Token::RightAngle))
		{
			auto saved_cursor = cursor;
			bool fall_back	  = should_fall_back_to_identifier();
			cursor			  = saved_cursor;
			if (fall_back)
			{
				cursor.retreat();
				return ast::Identifier {name};
			}

			do
				generic_args.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			cursor.advance();
		}
		return ast::GenericIdentifier {name, std::move(generic_args)};
	}

	bool should_fall_back_to_identifier()
	{
		bool saved	  = should_report;
		should_report = false;
		defer
		{
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
		if (cursor.match(Token::RightAngle))
		{
			auto kind = cursor.next_kind();
			return (contains(fallbacks, kind) && expr_depth != 0) || (open_angles == 1 && kind == Token::RightAngle);
		}
		return true;
	}

	template<ast::NumericLiteral (*EVALUATE)(std::string_view)>
	ExpressionNode on_numeric_literal()
	{
		auto lexeme = cursor.previous().get_lexeme(source);
		return EVALUATE(lexeme);
	}

	ExpressionNode on_string_literal()
	{
		auto lexeme = cursor.previous().get_lexeme(source);
		return evaluate_string_literal(lexeme);
	}

	ExpressionNode on_prefix_left_paren()
	{
		return parse_call({}); // TODO: function type
	}

	ExpressionNode on_prefix_left_bracket()
	{
		return parse_array_type(); // TODO: array literal
	}

	std::vector<Expression> parse_bracketed_arguments()
	{
		uint32_t saved = open_angles;
		open_angles	   = 0;
		defer
		{
			open_angles = saved;
		};

		std::vector<Expression> arguments;
		if (!cursor.match(Token::RightBracket))
		{
			do
				arguments.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			expect(Token::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return arguments;
	}

	ExpressionNode on_break()
	{
		return ast::Break {parse_optional_operand()};
	}

	ExpressionNode on_continue()
	{
		return ast::Continue {parse_optional_operand()};
	}

	ExpressionNode on_return()
	{
		return ast::Return {parse_optional_operand()};
	}

	ExpressionNode on_throw()
	{
		return ast::Throw {parse_optional_operand()};
	}

	OptionalExpression parse_optional_operand()
	{
		if (cursor.next_kind() == Token::Semicolon)
			return {};

		return parse_expression();
	}

	template<ast::UnaryOperator O>
	ExpressionNode on_prefix_operator()
	{
		auto right = parse_expression(Precedence::Prefix);
		return ast::UnaryExpression {O, right};
	}

	template<ast::BinaryOperator O, Precedence P>
	ExpressionNode on_infix_operator(Expression left)
	{
		auto target = cursor.previous();
		auto right	= parse_expression(P);
		validate_associativity(O, left, right, target);
		return ast::BinaryExpression {O, left, right};
	}

	template<ast::UnaryOperator O>
	ExpressionNode on_postfix_operator(Expression left)
	{
		return ast::UnaryExpression {O, left};
	}

	void validate_associativity(ast::BinaryOperator current, Expression left_index, Expression right_index, LexicalToken target)
	{
		auto& left_node	 = ast.get(left_index);
		auto& right_node = ast.get(right_index);

		if (auto right = std::get_if<ast::BinaryExpression>(&right_node))
			validate_binary_associativity(current, right->op, target);

		if (auto left = std::get_if<ast::BinaryExpression>(&left_node))
			validate_binary_associativity(left->op, current, target);
		else if (auto unary = std::get_if<ast::UnaryExpression>(&left_node))
			validate_unary_binary_associativity(unary->op, current, target);
	}

	void validate_binary_associativity(ast::BinaryOperator left, ast::BinaryOperator right, LexicalToken target)
	{
		bool chains = false;
		if (associates_arithmetic_and_bitwise(left, right) || associates_different_logical_operators(left, right)
			|| associates_comparison_operators(left, right, chains))
		{
			auto location = target.locate_in(source);
			if (chains)
				report(Message::AmbiguousOperatorChaining, location, BINARY_OPERATOR_STRINGS[left]);
			else
				report(Message::AmbiguousOperatorMixing, location, BINARY_OPERATOR_STRINGS[left],
					   BINARY_OPERATOR_STRINGS[right]);
		}
	}

	static bool associates_arithmetic_and_bitwise(ast::BinaryOperator left, ast::BinaryOperator right)
	{
		static constexpr ast::BinaryOperator bitwise_operators[] {ast::BinaryOperator::BitAnd, ast::BinaryOperator::BitOr,
																  ast::BinaryOperator::Xor, ast::BinaryOperator::LeftShift,
																  ast::BinaryOperator::RightShift};

		static constexpr ast::BinaryOperator arithmetic_operators[] {ast::BinaryOperator::Add,
																	 ast::BinaryOperator::Subtract,
																	 ast::BinaryOperator::Multiply,
																	 ast::BinaryOperator::Divide,
																	 ast::BinaryOperator::Remainder,
																	 ast::BinaryOperator::Power};

		return contains(bitwise_operators, left) ? contains(arithmetic_operators, right)
												 : contains(arithmetic_operators, left) && contains(bitwise_operators, right);
	}

	static bool associates_different_logical_operators(ast::BinaryOperator left, ast::BinaryOperator right)
	{
		return left == ast::BinaryOperator::LogicalAnd
				   ? right == ast::BinaryOperator::LogicalOr
				   : left == ast::BinaryOperator::LogicalOr && right == ast::BinaryOperator::LogicalAnd;
	}

	static bool associates_comparison_operators(ast::BinaryOperator left, ast::BinaryOperator right, bool& chains)
	{
		static constexpr ast::BinaryOperator comparison_operators[] {ast::BinaryOperator::Equal,
																	 ast::BinaryOperator::NotEqual,
																	 ast::BinaryOperator::Less,
																	 ast::BinaryOperator::Greater,
																	 ast::BinaryOperator::LessEqual,
																	 ast::BinaryOperator::GreaterEqual};

		chains = left == right;
		return contains(comparison_operators, left) && contains(comparison_operators, right);
	}

	void validate_unary_binary_associativity(ast::UnaryOperator left, ast::BinaryOperator right, LexicalToken target)
	{
		if (left == ast::UnaryOperator::Negate && right == ast::BinaryOperator::Power)
		{
			auto location = target.locate_in(source);
			report(Message::AmbiguousOperatorMixing, location, "-", "**");
		}
	}

	ExpressionNode on_dot(Expression left)
	{
		auto member = expect_name(Message::ExpectNameAfterDot);
		return ast::MemberAccess {left, member};
	}

	ExpressionNode on_infix_left_paren(Expression left)
	{
		return parse_call(OptionalExpression(left));
	}

	ExpressionNode parse_call(OptionalExpression callee)
	{
		uint32_t saved = open_angles;
		open_angles	   = 0;
		defer
		{
			open_angles = saved;
		};

		std::vector<Expression> arguments;
		if (!cursor.match(Token::RightParen))
		{
			do
				arguments.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectClosingParen);
		}
		return ast::Call {callee, std::move(arguments)};
	}

	ExpressionNode on_infix_left_bracket(Expression left)
	{
		return ast::Index {left, parse_bracketed_arguments()};
	}

	ExpressionNode on_caret()
	{
		return parse_pointer_type();
	}

	ast::Variability parse_variability()
	{
		auto specifier = ast::Variability::Specifier::Var;

		std::vector<Expression> arguments;
		if (cursor.match(Token::LeftBrace))
		{
			uint32_t saved = open_angles;
			open_angles	   = 0;
			defer
			{
				open_angles = saved;
			};

			specifier = ast::Variability::Specifier::VarBounded;
			if (!cursor.match(Token::RightBrace))
			{
				do
					arguments.emplace_back(parse_expression());
				while (cursor.match(Token::Comma));

				if (cursor.match(Token::Ellipsis))
					specifier = ast::Variability::Specifier::VarUnbounded;

				expect(Token::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		return {specifier, std::move(arguments)};
	}

	Expression parse_type()
	{
		const auto saved = expr_depth;
		expr_depth		 = 1;
		defer
		{
			expr_depth = saved;
		};

		if (cursor.match(Token::Caret))
			return ast.store(parse_pointer_type());
		if (cursor.match(Token::LeftBracket))
			return ast.store(parse_array_type());
		if (cursor.match(Token::LeftParen))
			return ast.store(parse_function_type());

		auto name = expect_name(Message::ExpectType);
		return ast.store(parse_identifier(name));
	}

	ExpressionNode parse_array_type()
	{
		auto bound = parse_expression();
		expect(Token::RightBracket, Message::ExpectBracketAfterArrayBound);
		auto type = parse_type();
		return ast::ArrayType {OptionalExpression(bound), type};
	}

	ExpressionNode parse_pointer_type()
	{
		ast::Variability variability;
		if (cursor.match(Token::Var))
			variability = parse_variability();

		auto type = parse_type();
		return ast::PointerType {variability, type};
	}

	ExpressionNode parse_function_type()
	{
		auto parameters = parse_function_type_parameters();
		expect(Token::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto outputs = parse_function_type_outputs();
		return ast::FunctionType {std::move(parameters), std::move(outputs)};
	}

	std::vector<ast::FunctionType::Parameter> parse_function_type_parameters()
	{
		std::vector<ast::FunctionType::Parameter> parameters;
		if (!cursor.match(Token::RightParen))
		{
			do
				parameters.emplace_back(parse_function_type_parameter());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	ast::FunctionType::Parameter parse_function_type_parameter()
	{
		auto specifier = ast::ParameterSpecifier::In;
		if (cursor.match(Token::Let))
			specifier = ast::ParameterSpecifier::Let;
		else if (cursor.match(Token::Var))
			specifier = ast::ParameterSpecifier::Var;

		auto type = parse_type();

		std::string_view name;
		if (auto name_token = cursor.match(Token::Name))
			name = name_token->get_lexeme(source);

		if (auto equal = cursor.match(Token::Equals))
		{
			auto location = equal->locate_in(source);
			report(Message::FuncTypeDefaultArgument, location);
			throw ParseError();
		}
		return {specifier, name, type};
	}

	std::vector<ast::FunctionOutput> parse_function_type_outputs()
	{
		std::vector<ast::FunctionOutput> outputs;
		if (cursor.match(Token::LeftParen))
		{
			do
				outputs.emplace_back(parse_function_output());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterOutputs);
		}
		else
		{
			auto type = parse_type();
			outputs.emplace_back(ast::FunctionOutput {type, std::string_view()});
		}

		return outputs;
	}

	void expect(Token kind, CheckedMessage<std::string> message)
	{
		auto token = cursor.next();
		if (token.kind == kind)
			cursor.advance();
		else
		{
			report_expectation(message, token);
			throw ParseError();
		}
	}

	std::string_view expect_name(CheckedMessage<std::string> message)
	{
		auto token = cursor.next();
		if (token.kind == Token::Name)
		{
			cursor.advance();
			return token.get_lexeme(source);
		}

		report_expectation(message, token);
		return {};
	}

	void report_expectation(CheckedMessage<std::string> message, LexicalToken unexpected)
	{
		auto location = unexpected.locate_in(source);
		report(message, location, unexpected.to_message_string(source));
	}

	template<typename... Args>
	void report(CheckedMessage<Args...> message, SourceLocation location, Args&&... args)
	{
		if (should_report)
			reporter.report(message, location, std::forward<Args>(args)...);
	}

	static constexpr LookupTable<Token, PrefixParse> PREFIX_PARSES = []
	{
		using enum Token;
		using enum ast::UnaryOperator;

		LookupTable<Token, PrefixParse> t;
		t[Name]			 = &Parser::on_name;
		t[If]			 = &Parser::on_if;
		t[While]		 = &Parser::on_while;
		t[For]			 = &Parser::on_for;
		t[LeftBrace]	 = &Parser::on_left_brace;
		t[Let]			 = &Parser::on_let;
		t[Var]			 = &Parser::on_var;
		t[Static]		 = &Parser::on_static;
		t[Const]		 = &Parser::on_const;
		t[DecIntLiteral] = &Parser::on_numeric_literal<evaluate_dec_int_literal>;
		t[HexIntLiteral] = &Parser::on_numeric_literal<evaluate_hex_int_literal>;
		t[BinIntLiteral] = &Parser::on_numeric_literal<evaluate_bin_int_literal>;
		t[OctIntLiteral] = &Parser::on_numeric_literal<evaluate_oct_int_literal>;
		t[FloatLiteral]	 = &Parser::on_numeric_literal<evaluate_float_literal>;
		t[CharLiteral]	 = &Parser::on_numeric_literal<evaluate_char_literal>;
		t[StringLiteral] = &Parser::on_string_literal;
		t[LeftParen]	 = &Parser::on_prefix_left_paren;
		t[LeftBracket]	 = &Parser::on_prefix_left_bracket;
		t[Break]		 = &Parser::on_break;
		t[Continue]		 = &Parser::on_continue;
		t[Return]		 = &Parser::on_return;
		t[Throw]		 = &Parser::on_throw;
		t[Ampersand]	 = &Parser::on_prefix_operator<AddressOf>;
		t[Minus]		 = &Parser::on_prefix_operator<Negate>;
		t[Bang]			 = &Parser::on_prefix_operator<LogicalNot>;
		t[Tilde]		 = &Parser::on_prefix_operator<BitwiseNot>;
		t[PlusPlus]		 = &Parser::on_prefix_operator<PreIncrement>;
		t[MinusMinus]	 = &Parser::on_prefix_operator<PreDecrement>;
		t[Caret]		 = &Parser::on_caret;
		return t;
	}();

	static constexpr LookupTable<Token, NonPrefixParseRule> NON_PREFIX_PARSES = []
	{
		using enum Token;
		using enum Precedence;
		using enum ast::UnaryOperator;
		using enum ast::BinaryOperator;

		LookupTable<Token, NonPrefixParseRule> t;
		t[Equals]				 = {Assignment, &Parser::on_infix_operator<Assign, Assignment>};
		t[PlusEquals]			 = {Assignment, &Parser::on_infix_operator<AddAssign, Assignment>};
		t[MinusEquals]			 = {Assignment, &Parser::on_infix_operator<SubtractAssign, Assignment>};
		t[StarEquals]			 = {Assignment, &Parser::on_infix_operator<MultiplyAssign, Assignment>};
		t[SlashEquals]			 = {Assignment, &Parser::on_infix_operator<DivideAssign, Assignment>};
		t[PercentEquals]		 = {Assignment, &Parser::on_infix_operator<RemainderAssign, Assignment>};
		t[StarStarEquals]		 = {Assignment, &Parser::on_infix_operator<PowerAssign, Assignment>};
		t[AmpersandEquals]		 = {Assignment, &Parser::on_infix_operator<AndAssign, Assignment>};
		t[PipeEquals]			 = {Assignment, &Parser::on_infix_operator<OrAssign, Assignment>};
		t[TildeEquals]			 = {Assignment, &Parser::on_infix_operator<XorAssign, Assignment>};
		t[LeftAngleAngleEquals]	 = {Assignment, &Parser::on_infix_operator<LeftShiftAssign, Assignment>};
		t[RightAngleAngleEquals] = {Assignment, &Parser::on_infix_operator<RightShiftAssign, Assignment>};
		t[AmpersandAmpersand]	 = {Logical, &Parser::on_infix_operator<LogicalAnd, Logical>};
		t[PipePipe]				 = {Logical, &Parser::on_infix_operator<LogicalOr, Logical>};
		t[EqualsEquals]			 = {Comparison, &Parser::on_infix_operator<Equal, Comparison>};
		t[BangEquals]			 = {Comparison, &Parser::on_infix_operator<NotEqual, Comparison>};
		t[LeftAngle]			 = {Comparison, &Parser::on_infix_operator<Less, Comparison>};
		t[LeftAngleEquals]		 = {Comparison, &Parser::on_infix_operator<LessEqual, Comparison>};
		t[RightAngleEquals]		 = {Comparison, &Parser::on_infix_operator<GreaterEqual, Comparison>};
		t[Plus]					 = {Additive, &Parser::on_infix_operator<Add, Additive>};
		t[Minus]				 = {Additive, &Parser::on_infix_operator<Subtract, Additive>};
		t[Ampersand]			 = {Bitwise, &Parser::on_infix_operator<BitAnd, Bitwise>};
		t[Pipe]					 = {Bitwise, &Parser::on_infix_operator<BitOr, Bitwise>};
		t[Tilde]				 = {Bitwise, &Parser::on_infix_operator<Xor, Bitwise>};
		t[LeftAngleAngle]		 = {Bitwise, &Parser::on_infix_operator<LeftShift, Bitwise>};
		t[Star]					 = {Multiplicative, &Parser::on_infix_operator<Multiply, Multiplicative>};
		t[Slash]				 = {Multiplicative, &Parser::on_infix_operator<Divide, Multiplicative>};
		t[Percent]				 = {Multiplicative, &Parser::on_infix_operator<Remainder, Multiplicative>};
		t[StarStar]				 = {Prefix, &Parser::on_infix_operator<Power, Multiplicative>}; // lower for right-associativity
		t[Caret]				 = {Postfix, &Parser::on_postfix_operator<Dereference>};
		t[PlusPlus]				 = {Postfix, &Parser::on_postfix_operator<PostIncrement>};
		t[MinusMinus]			 = {Postfix, &Parser::on_postfix_operator<PostDecrement>};
		t[Dot]					 = {Postfix, &Parser::on_dot};
		t[LeftParen]			 = {Postfix, &Parser::on_infix_left_paren};
		t[LeftBracket]			 = {Postfix, &Parser::on_infix_left_bracket};
		return t;
	}();
};

SyntaxTree parse(const Source& source, Reporter& reporter)
{
	auto tokens = lex(source, reporter);
	return parse(tokens, source, reporter);
}

SyntaxTree parse(const TokenStream& token_stream, const Source& source, Reporter& reporter)
{
	Parser parser(token_stream, source, reporter);
	return parser.parse();
}

} // namespace cero
