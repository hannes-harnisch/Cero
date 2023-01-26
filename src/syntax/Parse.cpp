#include "cero/syntax/Parse.hpp"

#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Literal.hpp"
#include "cero/util/LookupTable.hpp"
#include "syntax/ParseCursor.hpp"
#include "util/Defer.hpp"
#include "util/Fail.hpp"

namespace cero
{

class Parser
{
	SyntaxTree	  ast;
	const Source& source;
	Reporter&	  reporter;
	ParseCursor	  cursor;
	uint32_t	  unclosed_groups = 0;
	uint32_t	  unclosed_angles = 0;

	using PrefixParse	 = Expression (Parser::*)();
	using NonPrefixParse = Expression (Parser::*)(Expression);

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
		auto returns	= parse_return_values();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return ast.add(ast::Function {name, std::move(parameters), std::move(returns), std::move(statements)});
	}

	std::vector<ast::Function::Parameter> parse_function_definition_parameters()
	{
		++unclosed_groups;
		defer
		{
			--unclosed_groups;
		};

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
			default_argument = OptionalExpression(parse_expression());

		return {specifier, name, type, default_argument};
	}

	std::vector<ast::ReturnValue> parse_return_values()
	{
		std::vector<ast::ReturnValue> returns;
		if (cursor.match(Token::ThinArrow))
		{
			do
				returns.emplace_back(parse_return_value());
			while (cursor.match(Token::Comma));
		}
		return returns;
	}

	ast::ReturnValue parse_return_value()
	{
		auto type = parse_type();

		std::string_view name;
		if (auto token = cursor.match(Token::Name))
			name = token->get_lexeme(source);

		return {type, name};
	}

	std::vector<Expression> parse_block()
	{
		uint32_t saved_parens = unclosed_groups;
		uint32_t saved_angles = unclosed_angles;

		unclosed_groups = 0;
		unclosed_angles = 0;

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

		unclosed_angles = saved_angles;
		unclosed_groups = saved_parens;
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

		cursor.advance();
		auto left = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence))
		{
			auto right = (this->*parse)(left);
			// filter here

			left = right;
		}
		return left;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence precedence)
	{
		bool across_lines = cursor.is_next_new_line();
		auto token		  = cursor.next_breakable();

		if (precedence >= NON_PREFIX_PRECEDENCES[token.kind])
			return nullptr;

		if (across_lines && unclosed_groups == 0 && is_unbreakable_operator(token.kind))
			return nullptr;

		cursor.advance();
		return NON_PREFIX_PARSES[token.kind];
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

	Expression on_name()
	{
		auto name = cursor.previous().get_lexeme(source);
		return parse_identifier(name);
	}

	Expression parse_identifier(std::string_view name)
	{
		if (cursor.match(Token::LeftAngle))
			return parse_generic_identifier(name);

		return ast.add(ast::Identifier {name});
	}

	Expression parse_generic_identifier(std::string_view name)
	{
		++unclosed_angles;
		defer
		{
			--unclosed_angles;
		};

		std::vector<Expression> generic_args;
		if (!cursor.match(Token::RightAngle))
		{
			do
				generic_args.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
		} // Definitely unfinished
		return ast.add(ast::GenericIdentifier {name, std::move(generic_args)});
	}

	template<ast::NumericLiteral (*EVALUATE)(std::string_view)>
	Expression on_numeric_literal()
	{
		auto lexeme = cursor.previous().get_lexeme(source);
		return ast.add(EVALUATE(lexeme));
	}

	Expression on_string_literal()
	{
		auto lexeme = cursor.previous().get_lexeme(source);
		return ast.add(evaluate_string_literal(lexeme));
	}

	Expression on_let()
	{
		return ast.add(parse_binding(ast::Binding::Specifier::Let));
	}

	Expression on_var()
	{
		if (cursor.next_breakable().kind == Token::LeftBrace)
			return ast.add(parse_variability());

		return ast.add(parse_binding(ast::Binding::Specifier::Var));
	}

	Expression on_const()
	{
		return ast.add(parse_binding(ast::Binding::Specifier::Const));
	}

	Expression on_static()
	{
		auto specifier = ast::Binding::Specifier::Static;
		if (cursor.match(Token::Var))
			specifier = ast::Binding::Specifier::StaticVar;

		return ast.add(parse_binding(specifier));
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

	Expression on_prefix_left_brace()
	{
		return ast.add(ast::Block {parse_block()});
	}

	Expression on_prefix_left_paren()
	{
		return parse_call({});
	}

	Expression on_prefix_left_bracket()
	{
		return parse_array_type();
	}

	std::vector<Expression> parse_bracketed_arguments()
	{
		uint32_t saved	= unclosed_angles;
		unclosed_angles = 0;
		++unclosed_groups;
		defer
		{
			--unclosed_groups;
			unclosed_angles = saved;
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

	Expression on_if()
	{
		auto condition = parse_expression();
		expect_colon_or_block();
		auto then_expr = parse_expression();

		OptionalExpression else_expr;
		if (cursor.match(Token::Else))
			else_expr = OptionalExpression(parse_expression());

		return ast.add(ast::If {condition, then_expr, else_expr});
	}

	Expression on_while()
	{
		auto condition = parse_expression();
		expect_colon_or_block();
		auto statement = parse_expression();
		return ast.add(ast::WhileLoop {condition, statement});
	}

	Expression on_for()
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

	Expression on_break()
	{
		return ast.add(ast::Break {parse_optional_operand()});
	}

	Expression on_continue()
	{
		return ast.add(ast::Continue {parse_optional_operand()});
	}

	Expression on_return()
	{
		return ast.add(ast::Return {parse_optional_operand()});
	}

	Expression on_throw()
	{
		return ast.add(ast::Throw {parse_optional_operand()});
	}

	OptionalExpression parse_optional_operand()
	{
		if (cursor.is_next_new_line())
			return {};

		return parse_expression();
	}

	template<ast::UnaryOperator O, Precedence P>
	Expression on_prefix_operator()
	{
		auto right = parse_expression(P);
		return ast.add(ast::UnaryExpression {O, right});
	}

	template<ast::BinaryOperator O, Precedence P>
	Expression on_infix_operator(Expression left)
	{
		auto right = parse_expression(P);
		return ast.add(ast::BinaryExpression {O, left, right});
	}

	template<ast::UnaryOperator O>
	Expression on_postfix_operator(Expression left)
	{
		return ast.add(ast::UnaryExpression {O, left});
	}

	Expression on_dot(Expression left)
	{
		auto member = expect_name(Message::ExpectNameAfterDot);
		return ast.add(ast::MemberAccess {left, member});
	}

	Expression on_infix_left_paren(Expression left)
	{
		return parse_call(OptionalExpression(left));
	}

	Expression parse_call(OptionalExpression callee)
	{
		uint32_t saved	= unclosed_angles;
		unclosed_angles = 0;
		++unclosed_groups;
		defer
		{
			--unclosed_groups;
			unclosed_angles = saved;
		};

		std::vector<Expression> arguments;
		if (!cursor.match(Token::RightParen))
		{
			do
				arguments.emplace_back(parse_expression());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectClosingParen);
		}
		return ast.add(ast::Call {callee, std::move(arguments)});
	}

	Expression on_infix_left_bracket(Expression left)
	{
		return ast.add(ast::Index {left, parse_bracketed_arguments()});
	}

	Expression on_right_angle(Expression left)
	{
		if (unclosed_angles != 0)
		{
			cursor.retreat();
			return left;
		}

		auto right = parse_expression(Precedence::Comparison);
		return ast.add(ast::BinaryExpression {ast::BinaryOperator::Greater, left, right});
	}

	Expression on_right_angle_angle(Expression left)
	{
		auto right = parse_expression(Precedence::Bitwise);
		return ast.add(ast::BinaryExpression {ast::BinaryOperator::RightShift, left, right});
	}

	Expression on_caret()
	{
		return parse_pointer_type();
	}

	ast::Variability parse_variability()
	{
		auto specifier = ast::Variability::Specifier::Var;

		std::vector<Expression> arguments;
		if (cursor.match(Token::LeftBrace))
		{
			++unclosed_groups;
			defer
			{
				--unclosed_groups;
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
		if (cursor.match(Token::Caret))
			return parse_pointer_type();
		if (cursor.match(Token::LeftBracket))
			return parse_array_type();
		if (cursor.match(Token::LeftParen))
			return parse_function_type();

		auto name = expect_name(Message::ExpectType);
		return parse_identifier(name);
	}

	Expression parse_array_type()
	{
		auto bound = parse_expression();
		expect(Token::RightBracket, Message::ExpectBracketAfterArrayBound);
		auto type = parse_type();
		return ast.add(ast::ArrayType {OptionalExpression(bound), type});
	}

	Expression parse_pointer_type()
	{
		ast::Variability variability;
		if (cursor.match(Token::Var))
			variability = parse_variability();

		auto type = parse_type();
		return ast.add(ast::PointerType {variability, type});
	}

	Expression parse_function_type()
	{
		auto parameters = parse_function_type_parameters();
		expect(Token::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto returns = parse_function_type_returns();
		return ast.add(ast::FunctionType {std::move(parameters), std::move(returns)});
	}

	std::vector<ast::FunctionType::Parameter> parse_function_type_parameters()
	{
		++unclosed_groups;
		defer
		{
			--unclosed_groups;
		};

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

	std::vector<ast::ReturnValue> parse_function_type_returns()
	{
		std::vector<ast::ReturnValue> returns;
		if (cursor.match(Token::LeftParen))
		{
			++unclosed_groups;
			defer
			{
				--unclosed_groups;
			};

			do
				returns.emplace_back(parse_return_value());
			while (cursor.match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterReturns);
		}
		else
			returns.emplace_back(ast::ReturnValue {parse_type(), std::string_view()});

		return returns;
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
		t[RightAngle]			= &Parser::on_right_angle;
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
		t[RightAngleAngle]		= &Parser::on_right_angle_angle;
		t[StarStar]				= &Parser::on_infix_operator<Power, Multiplicative>;
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