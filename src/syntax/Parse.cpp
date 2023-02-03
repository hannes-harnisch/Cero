#include "cero/syntax/Parse.hpp"

#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Literal.hpp"
#include "cero/util/LookupTable.hpp"
#include "syntax/ParseCursor.hpp"
#include "util/Algorithm.hpp"
#include "util/Defer.hpp"
#include "util/Fail.hpp"

#include <utility>

namespace cero
{

class Parser
{
	SyntaxTree	  ast;
	const Source& source;
	Reporter&	  reporter;
	ParseCursor	  cursor;
	uint32_t	  expr_depth			  = 0;
	uint32_t	  open_groups			  = 0;
	uint32_t	  open_angles			  = 0;
	bool		  keep_double_right_angle = true;

	using PrefixParse	 = ExpressionNode (Parser::*)();
	using NonPrefixParse = ExpressionNode (Parser::*)(Expression);

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
		return std::move(ast);
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

	struct ParseError
	{};

	Definition parse_definition()
	{
		if (cursor.match(Token::Name))
			return parse_function();

		if (cursor.match(Token::Struct))
			return parse_struct();

		if (cursor.match(Token::Enum))
			return parse_enum();

		report_expectation(Message::ExpectFuncStructEnum, cursor.peek());
		throw ParseError();
	}

	void synchronize_definition()
	{
		auto kind = cursor.peek_kind();
		do
		{
			while (kind != Token::NewLine)
			{
				if (kind == Token::EndOfFile)
					return;

				cursor.advance();
				kind = cursor.peek_kind();
			}
			cursor.advance();
			kind = cursor.peek_kind();
		}
		while (kind != Token::Name && kind != Token::Struct && kind != Token::Enum && kind != Token::EndOfFile);
	}

	Definition parse_struct()
	{
		to_do();
	}

	Definition parse_enum()
	{
		to_do();
	}

	Definition parse_function()
	{
		auto name = cursor.previous().get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs	= parse_function_outputs();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return ast.store(ast::Function {name, std::move(parameters), std::move(outputs), std::move(statements)});
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
		if (cursor.match(Token::Equal))
		{
			++open_groups;
			defer
			{
				--open_groups;
			};
			default_argument = OptionalExpression(parse_expression());
		}
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
		uint32_t saved_groups	  = open_groups;
		uint32_t saved_angles	  = open_angles;
		bool	 saved_keep		  = keep_double_right_angle;
		expr_depth				  = 0;
		open_groups				  = 0;
		open_angles				  = 0;
		keep_double_right_angle	  = true;

		std::vector<Expression> statements;
		while (!cursor.match(Token::RightBrace))
		{
			try
			{
				statements.emplace_back(parse_expression());
			}
			catch (ParseError)
			{
				synchronize_statement();
			}
		}

		expr_depth				= saved_expr_depth;
		open_groups				= saved_groups;
		open_angles				= saved_angles;
		keep_double_right_angle = saved_keep;
		return statements;
	}

	Expression parse_expression(Precedence precedence = {})
	{
		auto next = cursor.next_breakable();

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
		auto left = ast.store((this->*parse_prefix)());
		while (auto parse = get_next_non_prefix_parse(precedence))
		{
			auto right = ast.store((this->*parse)(left));
			validate_associativity(left, right);
			left = right;
		}
		return left;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence precedence)
	{
		bool across_lines = cursor.is_next_new_line();
		auto kind		  = cursor.next_breakable().kind;

		if (precedence >= NON_PREFIX_PRECEDENCES[kind])
			return nullptr;

		if (across_lines && open_groups == 0 && is_unbreakable_operator(kind))
			return nullptr;

		if ((kind == Token::RightAngle || kind == Token::RightAngleAngle) && open_angles != 0)
			return nullptr;

		cursor.advance();
		return NON_PREFIX_PARSES[kind];
	}

	static bool is_unbreakable_operator(Token t)
	{
		return t == Token::LeftParen || t == Token::LeftBracket || t == Token::PlusPlus || t == Token::MinusMinus;
	}

	void synchronize_statement()
	{
		auto kind = cursor.peek_kind();
		while (kind != Token::NewLine && kind != Token::RightBrace && kind != Token::EndOfFile)
		{
			cursor.advance();
			kind = cursor.peek_kind();
		}
	}

	void validate_associativity(Expression, Expression)
	{}

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

		const auto saved = cursor;

		std::vector<Expression> generic_args;
		if (!cursor.match(Token::RightAngle))
		{
			do
				generic_args.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));

			static constexpr std::array fallbacks {Token::Name,			 Token::DecIntLiteral, Token::HexIntLiteral,
												   Token::BinIntLiteral, Token::OctIntLiteral, Token::FloatLiteral,
												   Token::CharLiteral,	 Token::StringLiteral, Token::Minus,
												   Token::Tilde,		 Token::Ampersand,	   Token::Bang,
												   Token::PlusPlus,		 Token::MinusMinus};
			if (cursor.match(Token::RightAngle))
			{
				auto kind = cursor.next_breakable().kind;
				if (contains(fallbacks, kind) && expr_depth != 0)
					return fall_back_to_identifier(saved, name);
			}
			else if (cursor.match(Token::RightAngleAngle))
			{
				auto kind = cursor.next_breakable().kind;
				if ((contains(fallbacks, kind) && expr_depth != 0) || (open_angles == 1 && keep_double_right_angle))
					return fall_back_to_identifier(saved, name);

				if (keep_double_right_angle)
					cursor.retreat_to_last_breakable();

				keep_double_right_angle ^= true;
			}
			else
				return fall_back_to_identifier(saved, name);
		}
		return ast::GenericIdentifier {name, std::move(generic_args)};
	}

	ExpressionNode fall_back_to_identifier(ParseCursor saved, std::string_view name)
	{
		cursor = std::move(saved);
		cursor.retreat_to_last_breakable();
		return ast::Identifier {name};
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

	ExpressionNode on_let()
	{
		return parse_binding(ast::Binding::Specifier::Let);
	}

	ExpressionNode on_var()
	{
		if (cursor.next_breakable().kind == Token::LeftBrace)
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
			if (cursor.match(Token::Equal))
			{
				auto name		 = name_token->get_lexeme(source);
				auto initializer = parse_expression();
				return {specifier, name, {}, OptionalExpression(initializer)};
			}
			cursor = saved;
		}

		auto type = parse_type();
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalExpression initializer;
		if (cursor.match(Token::Equal))
			initializer = OptionalExpression(parse_expression());

		return {specifier, name, OptionalExpression(type), initializer};
	}

	ExpressionNode on_prefix_left_brace()
	{
		return ast::Block {parse_block()};
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
		++open_groups;
		defer
		{
			--open_groups;
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

	ExpressionNode on_if()
	{
		auto condition = parse_expression();
		expect_colon_or_block();
		auto then_expr = parse_expression();

		OptionalExpression else_expr;
		if (cursor.match(Token::Else))
			else_expr = OptionalExpression(parse_expression());

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

	void expect_colon_or_block()
	{
		if (auto colon = cursor.match(Token::Colon))
		{
			auto next = cursor.next_breakable();
			if (next.kind == Token::LeftBrace)
				reporter.report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source));
		}
		else
		{
			auto next = cursor.next_breakable();
			if (next.kind != Token::LeftBrace)
				report_expectation(Message::ExpectColonOrBlock, next);
		}
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
		if (cursor.is_next_new_line())
			return {};

		return parse_expression();
	}

	template<ast::UnaryOperator O, Precedence P>
	ExpressionNode on_prefix_operator()
	{
		auto right = parse_expression(P);
		return ast::UnaryExpression {O, right};
	}

	template<ast::BinaryOperator O, Precedence P>
	ExpressionNode on_infix_operator(Expression left)
	{
		auto right = parse_expression(P);
		return ast::BinaryExpression {O, left, right};
	}

	template<ast::UnaryOperator O>
	ExpressionNode on_postfix_operator(Expression left)
	{
		return ast::UnaryExpression {O, left};
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
		++open_groups;
		defer
		{
			--open_groups;
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
		return ast.store(parse_type_node());
	}

	ExpressionNode parse_type_node()
	{
		const auto saved = expr_depth;
		expr_depth		 = 1;
		defer
		{
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

		if (auto equal = cursor.match(Token::Equal))
		{
			report(Message::FuncTypeDefaultArgument, *equal);
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
		auto token = cursor.next_breakable();
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
		auto token = cursor.next_breakable();
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
		reporter.report(message, location, unexpected.to_message_string(source));
	}

	void report(CheckedMessage<> message, LexicalToken unexpected)
	{
		auto location = unexpected.locate_in(source);
		reporter.report(message, location);
	}

	static constexpr LookupTable<Token, PrefixParse> PREFIX_PARSES = []
	{
		using enum Token;
		using enum Precedence;
		using enum ast::UnaryOperator;

		LookupTable<Token, PrefixParse> t(nullptr);
		t[Name]			 = &Parser::on_name;
		t[DecIntLiteral] = &Parser::on_numeric_literal<evaluate_dec_int_literal>;
		t[HexIntLiteral] = &Parser::on_numeric_literal<evaluate_hex_int_literal>;
		t[BinIntLiteral] = &Parser::on_numeric_literal<evaluate_bin_int_literal>;
		t[OctIntLiteral] = &Parser::on_numeric_literal<evaluate_oct_int_literal>;
		t[FloatLiteral]	 = &Parser::on_numeric_literal<evaluate_float_literal>;
		t[CharLiteral]	 = &Parser::on_numeric_literal<evaluate_char_literal>;
		t[StringLiteral] = &Parser::on_string_literal;
		t[Let]			 = &Parser::on_let;
		t[Var]			 = &Parser::on_var;
		t[Const]		 = &Parser::on_const;
		t[Static]		 = &Parser::on_static;
		t[LeftBrace]	 = &Parser::on_prefix_left_brace;
		t[LeftParen]	 = &Parser::on_prefix_left_paren;
		t[LeftBracket]	 = &Parser::on_prefix_left_bracket;
		t[If]			 = &Parser::on_if;
		t[While]		 = &Parser::on_while;
		t[For]			 = &Parser::on_for;
		t[Break]		 = &Parser::on_break;
		t[Continue]		 = &Parser::on_continue;
		t[Return]		 = &Parser::on_return;
		t[Throw]		 = &Parser::on_throw;
		t[Try]			 = &Parser::on_prefix_operator<TryOperator, Statement>;
		t[Ampersand]	 = &Parser::on_prefix_operator<AddressOf, Prefix>;
		t[Minus]		 = &Parser::on_prefix_operator<Negation, Prefix>;
		t[Bang]			 = &Parser::on_prefix_operator<LogicalNot, Prefix>;
		t[Tilde]		 = &Parser::on_prefix_operator<BitwiseNot, Prefix>;
		t[PlusPlus]		 = &Parser::on_prefix_operator<PreIncrement, Prefix>;
		t[MinusMinus]	 = &Parser::on_prefix_operator<PreDecrement, Prefix>;
		t[Caret]		 = &Parser::on_caret;
		return t;
	}();

	static constexpr LookupTable<Token, Precedence> NON_PREFIX_PRECEDENCES = []
	{
		using enum Token;
		using enum Precedence;

		LookupTable<Token, Precedence> t(Statement);
		t[Equal]				= Assignment;
		t[PlusEqual]			= Assignment;
		t[MinusEqual]			= Assignment;
		t[StarEqual]			= Assignment;
		t[SlashEqual]			= Assignment;
		t[PercentEqual]			= Assignment;
		t[StarStarEqual]		= Assignment;
		t[AmpersandEqual]		= Assignment;
		t[PipeEqual]			= Assignment;
		t[TildeEqual]			= Assignment;
		t[LeftAngleAngleEqual]	= Assignment;
		t[RightAngleAngleEqual] = Assignment;
		t[DoubleAmpersand]		= Logical;
		t[PipePipe]				= Logical;
		t[EqualEqual]			= Comparison;
		t[BangEqual]			= Comparison;
		t[LeftAngle]			= Comparison;
		t[RightAngle]			= Comparison;
		t[LeftAngleEqual]		= Comparison;
		t[RightAngleEqual]		= Comparison;
		t[Plus]					= Additive;
		t[Minus]				= Additive;
		t[Ampersand]			= Bitwise;
		t[Pipe]					= Bitwise;
		t[Tilde]				= Bitwise;
		t[LeftAngleAngle]		= Bitwise;
		t[RightAngleAngle]		= Bitwise;
		t[Star]					= Multiplicative;
		t[Slash]				= Multiplicative;
		t[Percent]				= Multiplicative;
		t[StarStar]				= Prefix;
		t[Dot]					= Postfix;
		t[ColonColon]			= Postfix;
		t[LeftParen]			= Postfix;
		t[LeftBracket]			= Postfix;
		t[Caret]				= Postfix;
		t[PlusPlus]				= Postfix;
		t[MinusMinus]			= Postfix;
		return t;
	}();

	static constexpr LookupTable<Token, NonPrefixParse> NON_PREFIX_PARSES = []
	{
		using enum Token;
		using enum Precedence;
		using enum ast::UnaryOperator;
		using enum ast::BinaryOperator;

		LookupTable<Token, NonPrefixParse> t(nullptr);
		t[Dot]					= &Parser::on_dot;
		t[LeftParen]			= &Parser::on_infix_left_paren;
		t[LeftBracket]			= &Parser::on_infix_left_bracket;
		t[Equal]				= &Parser::on_infix_operator<Assign, Assignment>;
		t[PlusEqual]			= &Parser::on_infix_operator<AddAssign, Assignment>;
		t[MinusEqual]			= &Parser::on_infix_operator<SubtractAssign, Assignment>;
		t[StarEqual]			= &Parser::on_infix_operator<MultiplyAssign, Assignment>;
		t[SlashEqual]			= &Parser::on_infix_operator<DivideAssign, Assignment>;
		t[PercentEqual]			= &Parser::on_infix_operator<RemainderAssign, Assignment>;
		t[StarStarEqual]		= &Parser::on_infix_operator<PowerAssign, Assignment>;
		t[AmpersandEqual]		= &Parser::on_infix_operator<BitAndAssign, Assignment>;
		t[PipeEqual]			= &Parser::on_infix_operator<BitOrAssign, Assignment>;
		t[TildeEqual]			= &Parser::on_infix_operator<XorAssign, Assignment>;
		t[LeftAngleAngleEqual]	= &Parser::on_infix_operator<LeftShiftAssign, Assignment>;
		t[RightAngleAngleEqual] = &Parser::on_infix_operator<RightShiftAssign, Assignment>;
		t[DoubleAmpersand]		= &Parser::on_infix_operator<LogicalAnd, Logical>;
		t[PipePipe]				= &Parser::on_infix_operator<LogicalOr, Logical>;
		t[EqualEqual]			= &Parser::on_infix_operator<Equality, Comparison>;
		t[BangEqual]			= &Parser::on_infix_operator<Inequality, Comparison>;
		t[LeftAngle]			= &Parser::on_infix_operator<Less, Comparison>;
		t[RightAngle]			= &Parser::on_infix_operator<Greater, Comparison>;
		t[LeftAngleEqual]		= &Parser::on_infix_operator<LessEqual, Comparison>;
		t[RightAngleEqual]		= &Parser::on_infix_operator<GreaterEqual, Comparison>;
		t[Plus]					= &Parser::on_infix_operator<Add, Additive>;
		t[Minus]				= &Parser::on_infix_operator<Subtract, Additive>;
		t[Star]					= &Parser::on_infix_operator<Multiply, Multiplicative>;
		t[Slash]				= &Parser::on_infix_operator<Divide, Multiplicative>;
		t[Percent]				= &Parser::on_infix_operator<Remainder, Multiplicative>;
		t[Ampersand]			= &Parser::on_infix_operator<BitAnd, Bitwise>;
		t[Pipe]					= &Parser::on_infix_operator<BitOr, Bitwise>;
		t[Tilde]				= &Parser::on_infix_operator<Xor, Bitwise>;
		t[LeftAngleAngle]		= &Parser::on_infix_operator<LeftShift, Bitwise>;
		t[RightAngleAngle]		= &Parser::on_infix_operator<RightShift, Bitwise>;
		t[StarStar]				= &Parser::on_infix_operator<Power, Multiplicative>; // one lower to make it right-associative
		t[Caret]				= &Parser::on_postfix_operator<Dereference>;
		t[PlusPlus]				= &Parser::on_postfix_operator<PostIncrement>;
		t[MinusMinus]			= &Parser::on_postfix_operator<PostDecrement>;
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
