#include "Parser.hpp"

#include "util/LookupTable.hpp"

struct ParseError : std::exception
{
	const char* what() const override
	{
		return "parse error";
	}
};

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

class Parser
{
	SyntaxTree		   ast;
	const TokenStream& tokens;
	const Source&	   source;
	Reporter&		   reporter;
	uint32_t		   token_index	   = 0;
	uint32_t		   nested_generics = 0;

	using PrefixParse	 = Expression (Parser::*)();
	using NonPrefixParse = Expression (Parser::*)(Expression);

public:
	Parser(const TokenStream& tokens, const Source& source, Reporter& reporter) :
		tokens(tokens),
		source(source),
		reporter(reporter)
	{}

	SyntaxTree parse()
	{
		while (!match(TokenKind::EndOfFile))
		{
			try
			{
				parse_definition();
			}
			catch (ParseError)
			{}
		}

		return std::move(ast);
	}

private:
	using enum TokenKind;

	void parse_definition()
	{
		if (auto token = match_any({Struct, Enum, Name}))
		{
			switch (token->kind)
			{
				case Name: parse_function(*token); return;
				case Struct: parse_struct(); return;
				case Enum: parse_enum(); return;
			}
		}
		report_expectation(Message::ExpectFuncStructEnum, peek());
	}

	void parse_struct()
	{
		to_do();
	}

	void parse_enum()
	{
		to_do();
	}

	void parse_function(Token token)
	{
		auto name = token.get_lexeme_from(source);
		expect(LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_parameter_list();
		auto returns	= parse_return_list();
		expect(LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = finish_block();
		ast.add(Function(name, std::move(parameters), std::move(returns), std::move(statements)));
	}

	std::vector<Parameter> parse_parameter_list()
	{
		std::vector<Parameter> parameters;
		if (!match(RightParen))
		{
			do
				parameters.emplace_back(parse_parameter());
			while (match(Comma));

			expect(RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	Parameter parse_parameter()
	{
		auto kind = ParameterKind::In;
		if (match(Let))
			kind = ParameterKind::Let;
		else if (match(Var))
			kind = ParameterKind::Var;

		auto type = parse_type();
		auto name = expect_name(Message::ExpectParamName);

		Expression default_argument;
		if (match(Equal))
			default_argument = parse_expression();

		return {kind, name, type, default_argument};
	}

	std::vector<ReturnValue> parse_return_list()
	{
		std::vector<ReturnValue> returns;
		if (match(ThinArrow))
		{
			do
				returns.emplace_back(parse_return_value());
			while (match(Comma));
		}
		return returns;
	}

	ReturnValue parse_return_value()
	{
		auto type = parse_type();

		std::string_view name;
		if (auto token = match(Name))
			name = token->get_lexeme_from(source);

		return {type, name};
	}

	std::vector<Expression> finish_block()
	{
		std::vector<Expression> statements;
		while (!match(RightBrace))
		{
			try
			{
				statements.emplace_back(parse_expression());
				// if (match(RightBrace))
				//	break;
				//// postfix check causes this to be skipped erroneously
				// expect(NewLine, Message::ExpectBraceOrLineAfterStatement);
			}
			catch (ParseError)
			{
				auto token = peek();
				while (token.kind != NewLine && token.kind != RightBrace)
				{
					++token_index;
					token = peek();
				}
			}
		}
		return statements;
	}

	Expression parse_expression(Precedence precedence = {})
	{
		auto token = next_breakable();
		++token_index;

		auto parse_prefix = PREFIX_PARSES[token.kind];
		if (parse_prefix == nullptr)
		{
			report_expectation(Message::ExpectExpr, token);
			throw ParseError();
		}

		auto expression = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence))
		{
			auto right = (this->*parse)(expression);
			// filter here
			expression = right;
		}
		return expression;
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence precedence)
	{
		bool across_lines = next_is_new_line();
		auto token		  = next_breakable();

		if (precedence < NON_PREFIX_PRECEDENCES[token.kind])
			return nullptr;

		if (across_lines && is_unbreakable_operator(token.kind))
			return nullptr;

		return NON_PREFIX_PARSES[token.kind];
	}

	static bool is_unbreakable_operator(TokenKind kind)
	{
		return kind == LeftParen || kind == LeftBracket || kind == PlusPlus || kind == MinusMinus;
	}

	Expression parse_identifier()
	{
		auto name = previous().get_lexeme_from(source);
		return ast.add(Identifier(name));
	}

	Expression parse_dec_int_literal()
	{
		to_do();
	}

	Expression parse_hex_int_literal()
	{
		to_do();
	}

	Expression parse_bin_int_literal()
	{
		to_do();
	}

	Expression parse_oct_int_literal()
	{
		to_do();
	}

	Expression parse_dec_float_literal()
	{
		to_do();
	}

	Expression parse_hex_float_literal()
	{
		to_do();
	}

	Expression parse_char_literal()
	{
		return {};
	}

	Expression parse_string_literal()
	{
		return {};
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
		uint32_t saved = token_index;
		if (auto name = match(Name))
		{
			if (match(Equal))
			{
				auto initializer = parse_expression();
				return {name->get_lexeme_from(source), {}, initializer};
			}
			token_index = saved;
		}

		auto type = parse_type();
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		Expression initializer;
		if (match(Equal))
			initializer = parse_expression();

		return {name, type, initializer};
	}

	Expression parse_block_expression()
	{
		return ast.add(BlockExpression(finish_block()));
	}

	Expression parse_parenthesized()
	{
		auto expression = parse_expression();
		expect(RightParen, Message::ExpectParenAfterExpr);
		return expression;
	}

	Expression parse_if()
	{
		auto condition = parse_expression();
		expect(Colon, Message::ExpectColonAfterCondition); // warn on colon with blocks
		auto then_expr = parse_expression();

		Expression else_expr;
		if (match(Else))
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
		if (!match(RightParen))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Comma));
			expect(RightParen, Message::ExpectParenAfterCall);
		}
		return ast.add(Call(left, std::move(arguments)));
	}

	Expression parse_indexing(Expression left)
	{
		std::vector<Expression> arguments;
		if (!match(RightBracket))
		{
			do
				arguments.emplace_back(parse_expression());
			while (match(Comma));
			expect(RightBracket, Message::ExpectBracketAfterIndex);
		}
		return ast.add(Indexing(left, std::move(arguments)));
	}

	Expression parse_type()
	{
		__debugbreak();
		to_do();
	}

	std::optional<Token> match(TokenKind kind)
	{
		auto token = next_breakable();
		if (token.kind == kind)
		{
			++token_index;
			return token;
		}
		return {};
	}

	std::optional<Token> match_any(std::initializer_list<TokenKind> kinds)
	{
		auto token = next_breakable();
		for (auto kind : kinds)
		{
			if (token.kind == kind)
			{
				++token_index;
				return token;
			}
		}
		return {};
	}

	void expect(TokenKind kind, CheckedMessage<std::string> message)
	{
		auto token = next_breakable();
		if (token.kind == kind)
			++token_index;
		else
			report_expectation(message, token);
	}

	std::string_view expect_name(CheckedMessage<std::string> message)
	{
		auto token = next_breakable();
		if (token.kind == Name)
		{
			++token_index;
			return token.get_lexeme_from(source);
		}

		report_expectation(message, token);
		return {};
	}

	Token next_breakable()
	{
		Token token = peek();
		while (token.kind == NewLine || token.kind == LineComment || token.kind == BlockComment)
		{
			++token_index;
			token = peek();
		}
		return token;
	}

	bool next_is_new_line()
	{
		Token token = peek();
		while (token.kind == LineComment || token.kind == BlockComment)
		{
			++token_index;
			token = peek();
		}
		return token.kind == NewLine;
	}

	Token peek() const
	{
		return tokens.at(token_index);
	}

	Token previous() const
	{
		return tokens.at(token_index - 1);
	}

	void report_expectation(CheckedMessage<std::string> message, Token unexpected)
	{
		auto location = unexpected.locate_in(source);
		reporter.report(message, location, unexpected.describe_for_message(source));
	}

	static constexpr LookupTable<TokenKind, PrefixParse> PREFIX_PARSES = []
	{
		using enum TokenKind;
		using enum Precedence;

		LookupTable<TokenKind, PrefixParse> t(nullptr);
		t[Name]			   = &Parser::parse_identifier;
		t[DecIntLiteral]   = &Parser::parse_dec_int_literal;
		t[HexIntLiteral]   = &Parser::parse_hex_int_literal;
		t[BinIntLiteral]   = &Parser::parse_bin_int_literal;
		t[OctIntLiteral]   = &Parser::parse_oct_int_literal;
		t[DecFloatLiteral] = &Parser::parse_dec_float_literal;
		t[HexFloatLiteral] = &Parser::parse_hex_float_literal;
		t[CharLiteral]	   = &Parser::parse_char_literal;
		t[StringLiteral]   = &Parser::parse_string_literal;
		t[Let]			   = &Parser::parse_let_binding;
		t[Var]			   = &Parser::parse_var_binding;
		t[LeftBrace]	   = &Parser::parse_block_expression;
		t[LeftParen]	   = &Parser::parse_parenthesized;
		t[If]			   = &Parser::parse_if;
		t[While]		   = &Parser::parse_while_loop;
		t[For]			   = &Parser::parse_for_loop;
		t[Break]		   = &Parser::parse_break;
		t[Continue]		   = &Parser::parse_continue;
		t[Return]		   = &Parser::parse_return;
		t[Throw]		   = &Parser::parse_throw;
		t[Try]			   = &Parser::parse_prefix<TryExpression, Statement>;
		t[Ampersand]	   = &Parser::parse_prefix<AddressOf, Prefix>;
		t[Minus]		   = &Parser::parse_prefix<Negation, Prefix>;
		t[Bang]			   = &Parser::parse_prefix<LogicalNot, Prefix>;
		t[Tilde]		   = &Parser::parse_prefix<BitwiseNot, Prefix>;
		t[PlusPlus]		   = &Parser::parse_prefix<PreIncrement, Prefix>;
		t[MinusMinus]	   = &Parser::parse_prefix<PreDecrement, Prefix>;
		return t;
	}();

	static constexpr LookupTable<TokenKind, Precedence> NON_PREFIX_PRECEDENCES = []
	{
		using enum Precedence;

		LookupTable<TokenKind, Precedence> t(Statement);
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

	static constexpr LookupTable<TokenKind, NonPrefixParse> NON_PREFIX_PARSES = []
	{
		using enum TokenKind;
		using enum Precedence;

		LookupTable<TokenKind, NonPrefixParse> t(nullptr);
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
		t[RightAngle]			 = &Parser::parse_infix<Greater, Comparison>;
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
		t[DoubleRightAngle]		 = &Parser::parse_infix<RightShift, Bitwise>;
		t[StarStar]				 = &Parser::parse_infix<Exponentiation, Multiplicative>;
		t[Caret]				 = &Parser::parse_postfix<Dereference>;
		t[PlusPlus]				 = &Parser::parse_postfix<PostIncrement>;
		t[MinusMinus]			 = &Parser::parse_postfix<PostDecrement>;
		return t;
	}();
};

SyntaxTree parse(const TokenStream& tokens, const Source& source, Reporter& reporter)
{
	Parser parser(tokens, source, reporter);
	return parser.parse();
}
