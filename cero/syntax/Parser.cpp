#include "Parser.hpp"

#include "syntax/Literal.hpp"
#include "util/Defer.hpp"
#include "util/LookupTable.hpp"

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
	Postfix,
	Primary
};

struct ParseError
{};

class Parser
{
	SyntaxTree		   ast;
	const TokenStream& token_stream;
	const Source&	   source;
	Reporter&		   reporter;
	uint32_t		   cursor		   = 0;
	uint32_t		   unclosed_parens = 0;
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
				parse_definition();
			}
			catch (ParseError)
			{
				synchronize_definition();
			}
		}
		return std::move(ast);
	}

private:
	void parse_definition()
	{
		if (match(Token::Name))
			parse_function();
		else if (match(Token::Struct))
			parse_struct();
		else if (match(Token::Enum))
			parse_enum();
		else
		{
			report_expectation(Message::ExpectFuncStructEnum, peek());
			throw ParseError();
		}
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

	void parse_struct()
	{
		to_do();
	}

	void parse_enum()
	{
		to_do();
	}

	void parse_function()
	{
		auto name = previous().get_lexeme(source);
		expect(Token::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_parameter_list();
		auto returns	= parse_return_list();
		expect(Token::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		ast.add(Function(name, std::move(parameters), std::move(returns), std::move(statements)));
	}

	std::vector<Parameter> parse_parameter_list()
	{
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

		Expression default_argument;
		if (match(Token::Equal))
			default_argument = parse_expression();

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
		uint32_t saved_parens = unclosed_parens;
		uint32_t saved_angles = unclosed_angles;

		unclosed_parens = 0;
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
		unclosed_parens = saved_parens;
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
		auto expression = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence))
		{
			expression = (this->*parse)(expression);
			// filter here
		}
		return expression;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence precedence)
	{
		bool across_lines = next_is_new_line();
		auto token		  = next_breakable();

		if (precedence >= NON_PREFIX_PRECEDENCES[token.kind])
			return nullptr;

		if (across_lines && unclosed_parens == 0 && is_unbreakable_operator(token.kind))
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

	Expression parse_name()
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
	Expression parse_numeric_literal()
	{
		auto lexeme = previous().get_lexeme(source);
		return ast.add(EVALUATE(lexeme));
	}

	Expression parse_string_literal()
	{
		auto lexeme = previous().get_lexeme(source);
		return ast.add(StringLiteral(lexeme));
	}

	Expression parse_let_binding()
	{
		return ast.add(parse_binding());
	}

	Expression parse_var_binding()
	{
		auto [name, type, initializer] = parse_binding();
		return ast.add(VarBinding(name, type, initializer));
	}

	LetBinding parse_binding()
	{
		uint32_t saved = cursor;
		if (auto name = match(Token::Name))
		{
			if (match(Token::Equal))
			{
				auto initializer = parse_expression();
				return {name->get_lexeme(source), {}, initializer};
			}
			cursor = saved;
		}

		auto type = parse_type();
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		Expression initializer;
		if (match(Token::Equal))
			initializer = parse_expression();

		return {name, type, initializer};
	}

	Expression parse_block_expression()
	{
		return ast.add(BlockExpression(parse_block()));
	}

	Expression parse_parenthesized()
	{
		uint32_t saved	= unclosed_angles;
		unclosed_angles = 0;
		++unclosed_parens;
		defer
		{
			--unclosed_parens;
			unclosed_angles = saved;
		};

		auto expression = parse_expression();
		expect(Token::RightParen, Message::ExpectParenAfterGroup);
		return expression;
	}

	Expression parse_if()
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

		Expression else_expr;
		if (match(Token::Else))
			else_expr = parse_expression();

		return ast.add(IfExpression(condition, then_expr, else_expr));
	}

	Expression parse_while_loop()
	{
		to_do();
	}

	Expression parse_for_loop()
	{
		to_do();
	}

	Expression parse_break()
	{
		return ast.add(BreakExpression());
	}

	Expression parse_continue()
	{
		return ast.add(ContinueExpression());
	}

	Expression parse_return()
	{
		return ast.add(ReturnExpression({parse_return_or_throw_operand()}));
	}

	Expression parse_throw()
	{
		return ast.add(ThrowExpression({parse_return_or_throw_operand()}));
	}

	Expression parse_return_or_throw_operand()
	{
		if (next_is_new_line())
			return Expression();

		return parse_expression();
	}

	template<typename E, Precedence P>
	Expression parse_prefix()
	{
		auto right = parse_expression(P);
		return ast.add(E({right}));
	}

	template<typename E, Precedence P>
	Expression parse_infix(Expression left)
	{
		auto right = parse_expression(P);
		return ast.add(E({left, right}));
	}

	template<typename E>
	Expression parse_postfix(Expression left)
	{
		return ast.add(E({left}));
	}

	Expression parse_access(Expression left)
	{
		auto member = expect_name(Message::ExpectNameAfterDot);
		return ast.add(Access(left, member));
	}

	Expression parse_call(Expression left)
	{
		std::vector<Expression> arguments;
		if (!match(Token::RightParen))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Token::Comma));
			expect(Token::RightParen, Message::ExpectParenAfterCall);
		}
		return ast.add(Call(left, std::move(arguments)));
	}

	Expression parse_indexing(Expression left)
	{
		std::vector<Expression> arguments;
		if (!match(Token::RightBracket))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Token::Comma));
			expect(Token::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return ast.add(Indexing(left, std::move(arguments)));
	}

	Expression parse_right_angle(Expression left)
	{
		if (unclosed_angles != 0)
		{
			--cursor;
			return left;
		}

		auto right = parse_expression(Precedence::Comparison);
		return ast.add(Greater({left, right}));
	}

	Expression parse_double_right_angle(Expression left)
	{
		auto right = parse_expression(Precedence::Bitwise);
		return ast.add(RightShift({left, right}));
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
		return ast.add(ArrayTypeExpression(count, type));
	}

	Expression parse_pointer_type()
	{
		auto var_specifier = VarSpecifier::None;
		if (match(Token::Var))
			var_specifier = VarSpecifier::VarDefault;

		auto type = parse_type();
		return ast.add(PointerTypeExpression(var_specifier, type, {}));
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
		using enum Precedence;

		LookupTable<Token, PrefixParse> t(nullptr);
		t[Name]			 = &Parser::parse_name;
		t[DecIntLiteral] = &Parser::parse_numeric_literal<evaluate_dec_int_literal>;
		t[HexIntLiteral] = &Parser::parse_numeric_literal<evaluate_hex_int_literal>;
		t[BinIntLiteral] = &Parser::parse_numeric_literal<evaluate_bin_int_literal>;
		t[OctIntLiteral] = &Parser::parse_numeric_literal<evaluate_oct_int_literal>;
		t[FloatLiteral]	 = &Parser::parse_numeric_literal<evaluate_float_literal>;
		t[CharLiteral]	 = &Parser::parse_numeric_literal<evaluate_char_literal>;
		t[StringLiteral] = &Parser::parse_string_literal;
		t[Let]			 = &Parser::parse_let_binding;
		t[Var]			 = &Parser::parse_var_binding;
		t[LeftBrace]	 = &Parser::parse_block_expression;
		t[LeftParen]	 = &Parser::parse_parenthesized;
		t[If]			 = &Parser::parse_if;
		t[While]		 = &Parser::parse_while_loop;
		t[For]			 = &Parser::parse_for_loop;
		t[Break]		 = &Parser::parse_break;
		t[Continue]		 = &Parser::parse_continue;
		t[Return]		 = &Parser::parse_return;
		t[Throw]		 = &Parser::parse_throw;
		t[Try]			 = &Parser::parse_prefix<TryExpression, Statement>;
		t[Ampersand]	 = &Parser::parse_prefix<AddressOf, Prefix>;
		t[Minus]		 = &Parser::parse_prefix<Negation, Prefix>;
		t[Bang]			 = &Parser::parse_prefix<LogicalNot, Prefix>;
		t[Tilde]		 = &Parser::parse_prefix<BitwiseNot, Prefix>;
		t[PlusPlus]		 = &Parser::parse_prefix<PreIncrement, Prefix>;
		t[MinusMinus]	 = &Parser::parse_prefix<PreDecrement, Prefix>;
		return t;
	}();

	static constexpr LookupTable<Token, Precedence> NON_PREFIX_PRECEDENCES = []
	{
		using enum Token;
		using enum Precedence;

		LookupTable<Token, Precedence> t(Statement);
		t[Equal]				 = Assignment;
		t[PlusEqual]			 = Assignment;
		t[MinusEqual]			 = Assignment;
		t[StarEqual]			 = Assignment;
		t[SlashEqual]			 = Assignment;
		t[PercentEqual]			 = Assignment;
		t[StarStarEqual]		 = Assignment;
		t[AmpersandEqual]		 = Assignment;
		t[PipeEqual]			 = Assignment;
		t[TildeEqual]			 = Assignment;
		t[DoubleLeftAngleEqual]	 = Assignment;
		t[DoubleRightAngleEqual] = Assignment;
		t[DoubleAmpersand]		 = Logical;
		t[PipePipe]				 = Logical;
		t[EqualEqual]			 = Comparison;
		t[BangEqual]			 = Comparison;
		t[LeftAngle]			 = Comparison;
		t[RightAngle]			 = Comparison;
		t[LeftAngleEqual]		 = Comparison;
		t[RightAngleEqual]		 = Comparison;
		t[Plus]					 = Additive;
		t[Minus]				 = Additive;
		t[Ampersand]			 = Bitwise;
		t[Pipe]					 = Bitwise;
		t[Tilde]				 = Bitwise;
		t[DoubleLeftAngle]		 = Bitwise;
		t[DoubleRightAngle]		 = Bitwise;
		t[Star]					 = Multiplicative;
		t[Slash]				 = Multiplicative;
		t[Percent]				 = Multiplicative;
		t[StarStar]				 = Prefix;
		t[Dot]					 = Postfix;
		t[ColonColon]			 = Postfix;
		t[LeftParen]			 = Postfix;
		t[LeftBracket]			 = Postfix;
		t[Caret]				 = Postfix;
		t[PlusPlus]				 = Postfix;
		t[MinusMinus]			 = Postfix;
		return t;
	}();

	static constexpr LookupTable<Token, NonPrefixParse> NON_PREFIX_PARSES = []
	{
		using enum Token;
		using enum Precedence;

		LookupTable<Token, NonPrefixParse> t(nullptr);
		t[Dot]					 = &Parser::parse_access;
		t[LeftParen]			 = &Parser::parse_call;
		t[LeftBracket]			 = &Parser::parse_indexing;
		t[Equal]				 = &Parser::parse_infix<::Assignment, Assignment>;
		t[PlusEqual]			 = &Parser::parse_infix<AdditionAssignment, Assignment>;
		t[MinusEqual]			 = &Parser::parse_infix<SubtractionAssignment, Assignment>;
		t[StarEqual]			 = &Parser::parse_infix<MultiplicationAssignment, Assignment>;
		t[SlashEqual]			 = &Parser::parse_infix<DivisionAssignment, Assignment>;
		t[PercentEqual]			 = &Parser::parse_infix<RemainderAssignment, Assignment>;
		t[StarStarEqual]		 = &Parser::parse_infix<ExponentiationAssignment, Assignment>;
		t[AmpersandEqual]		 = &Parser::parse_infix<BitwiseAndAssignment, Assignment>;
		t[PipeEqual]			 = &Parser::parse_infix<BitwiseOrAssignment, Assignment>;
		t[TildeEqual]			 = &Parser::parse_infix<XorAssignment, Assignment>;
		t[DoubleLeftAngleEqual]	 = &Parser::parse_infix<LeftShiftAssignment, Assignment>;
		t[DoubleRightAngleEqual] = &Parser::parse_infix<RightShiftAssignment, Assignment>;
		t[DoubleAmpersand]		 = &Parser::parse_infix<LogicalAnd, Logical>;
		t[PipePipe]				 = &Parser::parse_infix<LogicalOr, Logical>;
		t[EqualEqual]			 = &Parser::parse_infix<Equality, Comparison>;
		t[BangEqual]			 = &Parser::parse_infix<Inequality, Comparison>;
		t[LeftAngle]			 = &Parser::parse_infix<Less, Comparison>;
		t[RightAngle]			 = &Parser::parse_right_angle;
		t[LeftAngleEqual]		 = &Parser::parse_infix<LessEqual, Comparison>;
		t[RightAngleEqual]		 = &Parser::parse_infix<GreaterEqual, Comparison>;
		t[Plus]					 = &Parser::parse_infix<Addition, Additive>;
		t[Minus]				 = &Parser::parse_infix<Subtraction, Additive>;
		t[Star]					 = &Parser::parse_infix<Multiplication, Multiplicative>;
		t[Slash]				 = &Parser::parse_infix<Division, Multiplicative>;
		t[Percent]				 = &Parser::parse_infix<Remainder, Multiplicative>;
		t[Ampersand]			 = &Parser::parse_infix<BitwiseAnd, Bitwise>;
		t[Pipe]					 = &Parser::parse_infix<BitwiseOr, Bitwise>;
		t[Tilde]				 = &Parser::parse_infix<Xor, Bitwise>;
		t[DoubleLeftAngle]		 = &Parser::parse_infix<LeftShift, Bitwise>;
		t[DoubleRightAngle]		 = &Parser::parse_double_right_angle;
		t[StarStar]				 = &Parser::parse_infix<Exponentiation, Multiplicative>;
		t[Caret]				 = &Parser::parse_postfix<Dereference>;
		t[PlusPlus]				 = &Parser::parse_postfix<PostIncrement>;
		t[MinusMinus]			 = &Parser::parse_postfix<PostDecrement>;
		return t;
	}();
};

SyntaxTree parse(const TokenStream& token_stream, const Source& source, Reporter& reporter)
{
	Parser parser(token_stream, source, reporter);
	return parser.parse();
}
