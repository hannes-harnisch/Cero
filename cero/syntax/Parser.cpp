#include "Parser.hpp"

#include "syntax/Literal.hpp"
#include "util/Defer.hpp"
#include "util/LookupTable.hpp"

class Parser
{
	SyntaxTree		   ast;
	const TokenStream& token_stream;
	const Source&	   source;
	Reporter&		   reporter;
	uint32_t		   cursor		   = 0;
	uint32_t		   unclosed_groups = 0;
	uint32_t		   unclosed_angles = 0;

	using PrefixParse	 = Expression (Parser::*)();
	using NonPrefixParse = Expression (Parser::*)(Expression);

public:
	Parser(const TokenStream& token_stream, const Source& source, Reporter& reporter) :
		token_stream(token_stream),
		source(source),
		reporter(reporter)
	{}

	SyntaxTree parse()
	{
		while (!match(Token::EndOfFile))
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
		if (match(Token::Name))
			return parse_function();

		if (match(Token::Struct))
			return parse_struct();

		if (match(Token::Enum))
			return parse_enum();

		report_expectation(Message::ExpectFuncStructEnum, peek());
		throw ParseError();
	}

	void synchronize_definition()
	{
		auto kind = peek().kind;
		do
		{
			while (kind != Token::NewLine)
			{
				if (kind == Token::EndOfFile)
					return;

				advance();
				kind = peek().kind;
			}
			advance();
			kind = peek().kind;
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
		auto name = previous().get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_parameter_list();
		auto returns	= parse_return_list();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return ast.add(Function(name, std::move(parameters), std::move(returns), std::move(statements)));
	}

	std::vector<Parameter> parse_parameter_list()
	{
		++unclosed_groups;
		defer
		{
			--unclosed_groups;
		};

		std::vector<Parameter> parameters;
		if (!match(Token::RightParen))
		{
			do
				parameters.emplace_back(parse_parameter());
			while (match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	Parameter parse_parameter()
	{
		auto kind = ParameterKind::In;
		if (match(Token::Let))
			kind = ParameterKind::Let;
		else if (match(Token::Var))
			kind = ParameterKind::Var;

		auto type = parse_type();
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty())
			throw ParseError();

		OptionalExpression default_argument;
		if (match(Token::Equal))
			default_argument = OptionalExpression(parse_expression());

		return {kind, name, type, default_argument};
	}

	std::vector<ReturnValue> parse_return_list()
	{
		std::vector<ReturnValue> returns;
		if (match(Token::ThinArrow))
		{
			do
				returns.emplace_back(parse_return_value());
			while (match(Token::Comma));
		}
		return returns;
	}

	ReturnValue parse_return_value()
	{
		auto type = parse_type();

		std::string_view name;
		if (auto token = match(Token::Name))
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
		while (!match(Token::RightBrace))
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
		auto next = next_breakable();

		auto parse_prefix = PREFIX_PARSES[next.kind];
		if (parse_prefix == nullptr)
		{
			report_expectation(Message::ExpectExpr, next);
			throw ParseError();
		}

		advance();
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
		bool across_lines = next_is_new_line();
		auto token		  = next_breakable();

		if (precedence >= NON_PREFIX_PRECEDENCES[token.kind])
			return nullptr;

		if (across_lines && unclosed_groups == 0 && is_unbreakable_operator(token.kind))
			return nullptr;

		advance();
		return NON_PREFIX_PARSES[token.kind];
	}

	static bool is_unbreakable_operator(Token t)
	{
		return t == Token::LeftParen || t == Token::LeftBracket || t == Token::PlusPlus || t == Token::MinusMinus;
	}

	void synchronize_statement()
	{
		auto kind = peek().kind;
		while (kind != Token::NewLine && kind != Token::RightBrace && kind != Token::EndOfFile)
		{
			advance();
			kind = peek().kind;
		}
	}

	Expression on_name()
	{
		auto name = previous().get_lexeme(source);
		return parse_identifier(name);
	}

	Expression parse_identifier(std::string_view name)
	{
		if (match(Token::LeftAngle))
			return parse_generic_identifier(name);

		return ast.add(Identifier(name));
	}

	Expression parse_generic_identifier(std::string_view name)
	{
		++unclosed_angles;
		defer
		{
			--unclosed_angles;
		};

		std::vector<Expression> generic_args;
		if (!match(Token::RightAngle))
		{
			do
				generic_args.emplace_back(parse_expression());
			while (match(Token::Comma));
		} // Definitely unfinished
		return ast.add(GenericIdentifier(name, std::move(generic_args)));
	}

	template<NumericLiteral (*EVALUATE)(std::string_view)>
	Expression on_numeric_literal()
	{
		auto lexeme = previous().get_lexeme(source);
		return ast.add(EVALUATE(lexeme));
	}

	Expression on_string_literal()
	{
		auto lexeme = previous().get_lexeme(source);
		return ast.add(StringLiteral(lexeme));
	}

	Expression on_let()
	{
		return ast.add(parse_binding(Binding::Specifier::Let));
	}

	Expression on_var()
	{
		if (next_breakable().kind == Token::LeftBrace)
			return ast.add(parse_variability());

		return ast.add(parse_binding(Binding::Specifier::Var));
	}

	Expression on_const()
	{
		return ast.add(parse_binding(Binding::Specifier::Const));
	}

	Expression on_static()
	{
		auto specifier = Binding::Specifier::Static;
		if (match(Token::Var))
			specifier = Binding::Specifier::StaticVar;

		return ast.add(parse_binding(specifier));
	}

	Binding parse_binding(Binding::Specifier specifier)
	{
		uint32_t saved = cursor;
		if (auto name_token = match(Token::Name))
		{
			if (match(Token::Equal))
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
		if (match(Token::Equal))
			initializer = OptionalExpression(parse_expression());

		return {specifier, name, OptionalExpression(type), initializer};
	}

	Expression on_prefix_left_brace()
	{
		return ast.add(BlockExpression(parse_block()));
	}

	Expression on_prefix_left_paren()
	{
		return parse_call({});
	}

	Expression on_prefix_left_bracket()
	{
		auto arguments = parse_bracketed_arguments();
		return Expression(~0u);
	}

	Expression on_if()
	{
		auto condition = parse_expression();
		if (auto colon = match(Token::Colon))
		{
			auto next = next_breakable();
			if (next.kind == Token::LeftBrace)
				reporter.report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source));
		}
		else
		{
			auto next = next_breakable();
			if (next.kind != Token::LeftBrace)
				report_expectation(Message::ExpectColonAfterCondition, next);
		}
		auto then_expr = parse_expression();

		OptionalExpression else_expr;
		if (match(Token::Else))
			else_expr = OptionalExpression(parse_expression());

		return ast.add(IfExpression(condition, then_expr, else_expr));
	}

	Expression on_while()
	{
		to_do();
	}

	Expression on_for()
	{
		to_do();
	}

	Expression on_break()
	{
		return ast.add(BreakExpression(parse_optional_operand()));
	}

	Expression on_continue()
	{
		return ast.add(ContinueExpression(parse_optional_operand()));
	}

	Expression on_return()
	{
		return ast.add(ReturnExpression(parse_optional_operand()));
	}

	Expression on_throw()
	{
		return ast.add(ThrowExpression(parse_optional_operand()));
	}

	OptionalExpression parse_optional_operand()
	{
		if (next_is_new_line())
			return {};

		return OptionalExpression(parse_expression());
	}

	template<UnaryOperator O, Precedence P>
	Expression on_prefix_operator()
	{
		auto right = parse_expression(P);
		return ast.add(UnaryExpression(O, right));
	}

	template<BinaryOperator O, Precedence P>
	Expression on_infix_operator(Expression left)
	{
		auto right = parse_expression(P);
		return ast.add(BinaryExpression(O, left, right));
	}

	template<UnaryOperator O>
	Expression on_postfix_operator(Expression left)
	{
		return ast.add(UnaryExpression(O, left));
	}

	Expression on_dot(Expression left)
	{
		auto member = expect_name(Message::ExpectNameAfterDot);
		return ast.add(MemberAccess(left, member));
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
		if (!match(Token::RightParen))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Token::Comma));
			expect(Token::RightParen, Message::ExpectClosingParen);
		}
		return ast.add(CallExpression(callee, std::move(arguments)));
	}

	Expression on_infix_left_bracket(Expression left)
	{
		return ast.add(IndexExpression(left, parse_bracketed_arguments()));
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
		if (!match(Token::RightBracket))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Token::Comma));
			expect(Token::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return arguments;
	}

	Expression on_right_angle(Expression left)
	{
		if (unclosed_angles != 0)
		{
			--cursor;
			return left;
		}

		auto right = parse_expression(Precedence::Comparison);
		return ast.add(BinaryExpression(BinaryOperator::Greater, left, right));
	}

	Expression on_right_angle_angle(Expression left)
	{
		auto right = parse_expression(Precedence::Bitwise);
		return ast.add(BinaryExpression(BinaryOperator::RightShift, left, right));
	}

	Expression on_caret()
	{
		return parse_pointer_type();
	}

	Variability parse_variability()
	{
		auto specifier = Variability::Specifier::Var;

		std::vector<Expression> arguments;
		if (match(Token::LeftBrace))
		{
			specifier = Variability::Specifier::VarBounded;
			if (!match(Token::RightBrace))
			{
				do
					arguments.emplace_back(parse_expression());
				while (match(Token::Comma));

				if (match(Token::Ellipsis))
					specifier = Variability::Specifier::VarUnbounded;

				expect(Token::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		return {specifier, std::move(arguments)};
	}

	Expression parse_type()
	{
		if (match(Token::LeftBracket))
			return parse_array_type();
		if (match(Token::Caret))
			return parse_pointer_type();

		auto name = expect_name(Message::ExpectType);
		return parse_identifier(name);
	}

	Expression parse_array_type()
	{
		auto count = parse_expression();
		expect(Token::RightBracket, Message::ExpectBracketAfterArrayCount);
		auto type = parse_type();
		return ast.add(ArrayTypeExpression(OptionalExpression(count), type));
	}

	Expression parse_pointer_type()
	{
		Variability variability;
		if (match(Token::Var))
			variability = parse_variability();

		auto type = parse_type();
		return ast.add(PointerTypeExpression(variability, type));
	}

	std::optional<LexicalToken> match(Token kind)
	{
		auto token = next_breakable();
		if (token.kind == kind)
		{
			advance();
			return token;
		}
		return {};
	}

	void expect(Token kind, CheckedMessage<std::string> message)
	{
		auto token = next_breakable();
		if (token.kind == kind)
			advance();
		else
		{
			report_expectation(message, token);
			throw ParseError();
		}
	}

	std::string_view expect_name(CheckedMessage<std::string> message)
	{
		auto token = next_breakable();
		if (token.kind == Token::Name)
		{
			advance();
			return token.get_lexeme(source);
		}

		report_expectation(message, token);
		return {};
	}

	LexicalToken next_breakable()
	{
		auto token = peek();
		while (token.kind == Token::NewLine || token.kind == Token::LineComment || token.kind == Token::BlockComment)
		{
			advance();
			token = peek();
		}
		return token;
	}

	bool next_is_new_line()
	{
		auto kind = peek().kind;
		while (kind == Token::LineComment || kind == Token::BlockComment)
		{
			advance();
			kind = peek().kind;
		}
		return kind == Token::NewLine;
	}

	void advance()
	{
		++cursor;
	}

	LexicalToken peek() const
	{
		return token_stream.at(cursor);
	}

	LexicalToken previous() const
	{
		return token_stream.at(cursor - 1);
	}

	void report_expectation(CheckedMessage<std::string> message, LexicalToken unexpected)
	{
		auto location = unexpected.locate_in(source);
		reporter.report(message, location, unexpected.to_message_string(source));
	}

	static constexpr LookupTable<Token, PrefixParse> PREFIX_PARSES = []
	{
		using enum Token;
		using enum UnaryOperator;
		using enum Precedence;

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
		using enum UnaryOperator;
		using enum BinaryOperator;
		using enum Precedence;

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

SyntaxTree parse(const TokenStream& token_stream, const Source& source, Reporter& reporter)
{
	Parser parser(token_stream, source, reporter);
	return parser.parse();
}
