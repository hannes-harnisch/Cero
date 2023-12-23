#include "Parse.hpp"

#include "cero/syntax/Lex.hpp"
#include "cero/syntax/Literal.hpp"
#include "cero/syntax/TokenCursor.hpp"
#include "cero/util/Algorithm.hpp"
#include "cero/util/Defer.hpp"
#include "cero/util/Fail.hpp"

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
	Parser(const TokenStream& token_stream, const SourceLock& source, Reporter& reporter) :
		source_(source),
		reporter_(reporter),
		cursor_(token_stream) {
	}

	Ast parse() {
		std::vector<AstNode> definitions;
		while (!cursor_.match(TokenKind::EndOfFile)) {
			try {
				definitions.emplace_back(parse_definition());
			} catch (ParseError) {
				recover_at_definition_scope();
			}
		}
		auto defs = ast_.store_multiple(definitions);
		ast_.store(AstRoot {{0}, defs});
		return std::move(ast_);
	}

private:
	Ast ast_;
	const SourceLock& source_;
	Reporter& reporter_;
	TokenCursor cursor_;
	bool is_looking_ahead_ = false;
	bool is_binding_allowed_ = true;
	uint32_t open_angles_ = 0;

	using PrefixParse = AstNode (Parser::*)();
	using NonPrefixParse = AstNode (Parser::*)(AstId left);

	struct NonPrefixParseRule {
		Precedence precedence = {};
		NonPrefixParse func = nullptr;
	};

	AstNode parse_definition() {
		auto access_specifier = AccessSpecifier::None;
		if (cursor_.match(TokenKind::Private)) {
			access_specifier = AccessSpecifier::Private;
		} else if (cursor_.match(TokenKind::Public)) {
			access_specifier = AccessSpecifier::Public;
		}

		if (auto name_token = cursor_.match_name()) {
			return parse_function(access_specifier, *name_token);
		}

		if (cursor_.match(TokenKind::Struct)) {
			return parse_struct(access_specifier);
		}

		if (cursor_.match(TokenKind::Enum)) {
			return parse_enum(access_specifier);
		}

		report_expectation(Message::ExpectFuncStructEnum, cursor_.current());
		throw ParseError();
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

	AstNode parse_struct(AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForStruct);
		return {AstStructDefinition {{0}, access_specifier, name}};
	}

	AstNode parse_enum(AccessSpecifier access_specifier) {
		auto name = expect_name(Message::ExpectNameForEnum);
		return {AstEnumDefinition {{0}, access_specifier, name}};
	}

	AstNode parse_function(AccessSpecifier access_specifier, Token name_token) {
		auto name = name_token.get_lexeme(source_);
		expect(TokenKind::LeftParen, Message::ExpectParenAfterFuncName);

		auto parameters = parse_function_definition_parameters();
		auto outputs = parse_function_definition_outputs();
		expect(TokenKind::LeftBrace, Message::ExpectBraceBeforeFuncBody);

		auto statements = parse_block();
		return {AstFunctionDefinition {
			{0},
			access_specifier,
			name,
			std::move(parameters),
			std::move(outputs),
			statements,
		}};
	}

	std::vector<AstFunctionParameter> parse_function_definition_parameters() {
		std::vector<AstFunctionParameter> parameters;
		if (!cursor_.match(TokenKind::RightParen)) {
			do {
				parameters.emplace_back(parse_function_definition_parameter());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionParameter parse_function_definition_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor_.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor_.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto type = ast_.store(parse_type());
		auto name = expect_name(Message::ExpectParamName);
		if (name.empty()) {
			throw ParseError(); // TODO: explain why this is necessary
		}

		OptionalAstId default_argument;
		if (cursor_.match(TokenKind::Equals)) {
			default_argument = ast_.store(parse_subexpression());
		}

		return {{0}, specifier, type, name, default_argument};
	}

	std::vector<AstFunctionOutput> parse_function_definition_outputs() {
		std::vector<AstFunctionOutput> outputs;
		if (cursor_.match(TokenKind::ThinArrow)) {
			do {
				outputs.emplace_back(parse_function_definition_output());
			} while (cursor_.match(TokenKind::Comma));
		}
		return outputs;
	}

	AstFunctionOutput parse_function_definition_output() {
		auto type = ast_.store(parse_type());

		StringId name;
		if (auto name_token = cursor_.match_name()) {
			name = name_token->get_lexeme(source_);
		}

		return {{0}, type, name};
	}

	AstIdSet parse_block() {
		const uint32_t saved_angles = std::exchange(open_angles_, 0);
		const bool saved_binding_allowed = std::exchange(is_binding_allowed_, true);

		std::vector<AstNode> statements;
		while (!cursor_.match(TokenKind::RightBrace)) {
			try {
				statements.emplace_back(parse_statement());
			} catch (ParseError) {
				bool at_end = recover_at_statement_scope();
				if (at_end) {
					break;
				}
			}
		}

		open_angles_ = saved_angles;
		is_binding_allowed_ = saved_binding_allowed;
		return ast_.store_multiple(statements);
	}

	bool recover_at_statement_scope() {
		TokenKind kind = cursor_.current_kind();
		while (kind != TokenKind::EndOfFile) {
			if (kind == TokenKind::Semicolon) {
				cursor_.advance();
				return false;
			}
			if (kind == TokenKind::RightBrace) {
				return false;
			}

			cursor_.advance();
			kind = cursor_.current_kind();
		}
		return true;
	}

	AstNode parse_statement() {
		bool parses_complete_stmt = false;
		auto parse = lookup_statement_parse(parses_complete_stmt);
		auto stmt = (this->*parse)();

		if (!parses_complete_stmt) {
			if (auto name_token = cursor_.match_name()) {
				stmt = on_trailing_name(std::move(stmt), *name_token);
			}
			expect(TokenKind::Semicolon, Message::ExpectSemicolon);
		}
		return stmt;
	}

	PrefixParse lookup_statement_parse(bool& parses_complete_stmt) {
		auto next = cursor_.peek_kind();
		switch (next) {
			using enum TokenKind;
			case If: parses_complete_stmt = true; return &Parser::on_if_stmt;
			case For: parses_complete_stmt = true; return &Parser::on_for;
			case While: parses_complete_stmt = true; return &Parser::on_while;
			case LeftBrace: parses_complete_stmt = true; return &Parser::on_left_brace;
			case Let: return &Parser::on_let;
			case Var: return &Parser::on_var;
			case Const: return &Parser::on_const;
			case Static: return &Parser::on_static;
			default: return &Parser::parse_expression_or_binding;
		}
	}

	AstNode on_trailing_name(AstNode left_expr, Token name_token) {
		auto type = left_expr.get_kind();

		static constexpr AstNodeKind type_expr_kinds[] {AstNodeKind::NameExpr,		  AstNodeKind::GenericNameExpr,
														AstNodeKind::MemberExpr,	  AstNodeKind::GenericMemberExpr,
														AstNodeKind::ArrayTypeExpr,	  AstNodeKind::PointerTypeExpr,
														AstNodeKind::FunctionTypeExpr};
		if (!contains(type_expr_kinds, type)) {
			report_expectation(Message::ExpectSemicolon, name_token);
			throw ParseError();
		}

		auto name = name_token.get_lexeme(source_);
		auto left = ast_.store(std::move(left_expr));

		OptionalAstId initializer;
		if (cursor_.match(TokenKind::Equals)) {
			initializer = ast_.store(parse_subexpression());
		}

		return {AstBindingStatement {{0}, BindingSpecifier::Let, left, name, initializer}};
	}

	AstNode parse_expression_or_binding() {
		const bool saved_is_binding_allowed = std::exchange(is_binding_allowed_, true);
		Defer _ = [&] {
			is_binding_allowed_ = saved_is_binding_allowed;
		};
		return parse_expression(Precedence::Statement);
	}

	AstNode parse_subexpression(Precedence precedence = Precedence::Statement) {
		const bool saved_is_binding_allowed = std::exchange(is_binding_allowed_, false);
		Defer _ = [&] {
			is_binding_allowed_ = saved_is_binding_allowed;
		};
		return parse_expression(precedence);
	}

	AstNode parse_expression(Precedence precedence) {
		auto next = cursor_.peek();

		auto parse_prefix = lookup_prefix_parse(next.header.kind);
		if (parse_prefix == nullptr) {
			report_expectation(Message::ExpectExpr, next);
			throw ParseError();
		}

		auto expression = (this->*parse_prefix)();
		while (auto parse = get_next_non_prefix_parse(precedence)) {
			auto left = ast_.store(std::move(expression));
			expression = (this->*parse)(left);
		}

		return expression;
	}

	static PrefixParse lookup_prefix_parse(TokenKind kind) {
		switch (kind) {
			using enum TokenKind;
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
			case Ampersand: return &Parser::on_prefix_operator<UnaryOperator::AddressOf>;
			case Minus: return &Parser::on_prefix_operator<UnaryOperator::Negate>;
			case Bang: return &Parser::on_prefix_operator<UnaryOperator::LogicalNot>;
			case Tilde: return &Parser::on_prefix_operator<UnaryOperator::BitwiseNot>;
			case PlusPlus: return &Parser::on_prefix_operator<UnaryOperator::PreIncrement>;
			case MinusMinus: return &Parser::on_prefix_operator<UnaryOperator::PreDecrement>;
			case Caret: return &Parser::on_caret;
			default: return nullptr;
		}
	}

	NonPrefixParse get_next_non_prefix_parse(Precedence current_precedence) {
		auto token = cursor_.peek();

		if (token.header.kind == TokenKind::RightAngle && open_angles_ > 0) {
			return nullptr;
		}

		auto rule = select_non_prefix_parse(token);
		if (current_precedence >= rule.precedence) {
			return nullptr;
		}

		return rule.func;
	}

	NonPrefixParseRule select_non_prefix_parse(Token current) {
		if (current.header.kind != TokenKind::RightAngle) {
			return lookup_non_prefix_parse(current.header.kind);
		}

		auto next = cursor_.peek_ahead();
		if (next.header.kind == TokenKind::RightAngle && next.header.offset == current.header.offset + 1) {
			cursor_.advance();
			return {Precedence::Bitwise, &Parser::on_infix_operator<BinaryOperator::RightShift>};
		} else {
			return {Precedence::Comparison, &Parser::on_infix_operator<BinaryOperator::Greater>};
		}
	}

	static NonPrefixParseRule lookup_non_prefix_parse(TokenKind kind) {
		using enum Precedence;
		using enum UnaryOperator;
		using enum BinaryOperator;

		switch (kind) {
			using enum TokenKind;
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
		cursor_.advance();
		auto condition = ast_.store(parse_expression_or_binding());
		expect_colon_or_block();

		auto then_stmt = ast_.store(parse_statement());

		OptionalAstId else_stmt;
		if (cursor_.match(TokenKind::Else)) {
			else_stmt = ast_.store(parse_statement());
		}

		return {AstIfExpr {{0}, condition, then_stmt, else_stmt}};
	}

	AstNode on_if_expr() {
		cursor_.advance();
		auto condition = ast_.store(parse_expression_or_binding());
		expect(TokenKind::Colon, Message::ExpectColonInIfExpr);
		auto then_expr = ast_.store(parse_subexpression());
		expect(TokenKind::Else, Message::ExpectElse);
		auto else_expr = ast_.store(parse_subexpression());

		return {AstIfExpr {{0}, condition, then_expr, else_expr}};
	}

	AstNode on_while() {
		cursor_.advance();
		auto condition = ast_.store(parse_expression_or_binding());
		expect_colon_or_block();
		auto statement = ast_.store(parse_statement());
		return {AstWhileLoop {{0}, condition, statement}};
	}

	AstNode on_for() {
		cursor_.advance();
		to_do();
	}

	AstNode on_left_brace() {
		cursor_.advance();
		auto statements = parse_block();
		return {AstBlockStatement {{0}, statements}};
	}

	void expect_colon_or_block() {
		if (auto colon = cursor_.match_token(TokenKind::Colon)) {
			auto next = cursor_.peek();
			if (next.header.kind == TokenKind::LeftBrace) {
				report(Message::UnnecessaryColonBeforeBlock, colon->locate_in(source_));
			}
		} else {
			auto next = cursor_.peek();
			if (next.header.kind != TokenKind::LeftBrace) {
				report_expectation(Message::ExpectColonOrBlock, next);
			}
		}
	}

	AstNode on_let() {
		cursor_.advance();
		auto name = expect_name(Message::ExpectNameAfterLet);

		OptionalAstId initializer;
		if (cursor_.match(TokenKind::Equals)) {
			initializer = ast_.store(parse_subexpression());
		}

		return {AstBindingStatement {{0}, BindingSpecifier::Let, {}, name, initializer}};
	}

	AstNode on_var() {
		cursor_.advance();
		if (cursor_.peek_kind() == TokenKind::LeftBrace) {
			return {parse_variability()};
		}

		return {parse_binding(BindingSpecifier::Var)};
	}

	AstNode on_const() {
		cursor_.advance();
		return {parse_binding(BindingSpecifier::Const)};
	}

	AstNode on_static() {
		cursor_.advance();

		auto specifier = BindingSpecifier::Static;
		if (cursor_.match(TokenKind::Var)) {
			specifier = BindingSpecifier::StaticVar;
		}

		return {parse_binding(specifier)};
	}

	AstBindingStatement parse_binding(BindingSpecifier specifier) {
		auto lookahead = cursor_;
		if (auto name_token = lookahead.match_name()) {
			if (lookahead.match(TokenKind::Equals)) {
				cursor_ = lookahead;
				auto name = name_token->get_lexeme(source_);
				auto initializer = ast_.store(parse_subexpression());
				return {{0}, specifier, {}, name, initializer};
			}
		}

		auto type = ast_.store(parse_type());
		auto name = expect_name(Message::ExpectNameAfterDeclType);

		OptionalAstId initializer;
		if (cursor_.match(TokenKind::Equals)) {
			initializer = ast_.store(parse_subexpression());
		}

		return {{0}, specifier, type, name, initializer};
	}

	AstNode on_name() {
		auto name = cursor_.next().value().get_lexeme(source_);
		return parse_name(name);
	}

	AstNode parse_name(std::string_view name) {
		auto saved_cursor = cursor_;
		if (cursor_.match(TokenKind::LeftAngle)) {
			return parse_generic_name(name, saved_cursor);
		}

		return {AstNameExpr {{0}, name}};
	}

	AstNode parse_generic_name(std::string_view name, TokenCursor name_start) {
		++open_angles_;
		Defer _ = [&] {
			--open_angles_;
		};

		std::vector<AstNode> generic_args;
		if (!cursor_.match(TokenKind::RightAngle)) {
			size_t saved_node_count = ast_.ast_nodes_.size();

			bool fall_back = should_fall_back_to_name();

			cursor_ = name_start;
			rescind_lookahead(saved_node_count);

			if (fall_back) {
				return {AstNameExpr {{0}, name}};
			}

			cursor_.advance();
			do {
				generic_args.emplace_back(parse_subexpression());
			} while (cursor_.match(TokenKind::Comma));
			cursor_.advance();
		}

		auto arguments = ast_.store_multiple(generic_args);
		return {AstGenericNameExpr {{0}, name, arguments}};
	}

	bool should_fall_back_to_name() {
		const bool saved = std::exchange(is_looking_ahead_, true);
		Defer _ = [&] {
			is_looking_ahead_ = saved;
		};

		do {
			parse_subexpression();
		} while (cursor_.match(TokenKind::Comma));

		static constexpr TokenKind fallbacks[] {TokenKind::DecIntLiteral, TokenKind::HexIntLiteral, TokenKind::BinIntLiteral,
												TokenKind::OctIntLiteral, TokenKind::FloatLiteral,	TokenKind::CharLiteral,
												TokenKind::StringLiteral, TokenKind::Minus,			TokenKind::Tilde,
												TokenKind::Ampersand,	  TokenKind::Bang,			TokenKind::PlusPlus,
												TokenKind::MinusMinus};
		if (cursor_.match(TokenKind::RightAngle)) {
			auto kind = cursor_.peek_kind();
			return (kind == TokenKind::Name && !is_binding_allowed_) || contains(fallbacks, kind)
				   || (open_angles_ == 1 && kind == TokenKind::RightAngle);
		}
		return true;
	}

	template<AstNumericLiteralExpr (*EVALUATE)(std::string_view)>
	AstNode on_numeric_literal() {
		auto lexeme = cursor_.next().value().get_lexeme(source_);
		return {EVALUATE(lexeme)};
	}

	AstNode on_string_literal() {
		auto lexeme = cursor_.next().value().get_lexeme(source_);
		return {evaluate_string_literal(lexeme)};
	}

	AstNode on_prefix_left_paren() { // TODO: function type
		const uint32_t saved = std::exchange(open_angles_, 0);
		Defer _ = [&] {
			open_angles_ = saved;
		};

		cursor_.advance();

		std::vector<AstNode> arguments;
		if (!cursor_.match(TokenKind::RightParen)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectClosingParen);
		}
		auto args = ast_.store_multiple(arguments);
		return {AstGroupExpr {{0}, args}};
	}

	AstNode on_prefix_left_bracket() {
		return parse_array_type(); // TODO: array literal
	}

	AstIdSet parse_bracketed_arguments() {
		const uint32_t saved = std::exchange(open_angles_, 0);
		Defer _ = [&] {
			open_angles_ = saved;
		};

		std::vector<AstNode> arguments;
		if (!cursor_.match(TokenKind::RightBracket)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightBracket, Message::ExpectBracketAfterIndex);
		}
		return ast_.store_multiple(arguments);
	}

	AstNode on_break() {
		cursor_.advance();
		auto label = parse_optional_subexpression();
		return {AstBreakExpr {{0}, label}};
	}

	AstNode on_continue() {
		cursor_.advance();
		auto label = parse_optional_subexpression();
		return {AstContinueExpr {{0}, label}};
	}

	AstNode on_return() {
		cursor_.advance();
		std::vector<AstNode> values;
		if (expression_may_follow()) {
			do {
				values.emplace_back(parse_subexpression());
			} while (cursor_.match(TokenKind::Comma));
		}

		auto return_values = ast_.store_multiple(values);
		return {AstReturnExpr {{0}, return_values}};
	}

	AstNode on_throw() {
		cursor_.advance();
		auto expr = parse_optional_subexpression();
		return {AstThrowExpr {{0}, expr}};
	}

	OptionalAstId parse_optional_subexpression() {
		if (expression_may_follow()) {
			return ast_.store(parse_subexpression());
		}

		return {};
	}

	bool expression_may_follow() {
		auto next = cursor_.peek_kind();
		return lookup_prefix_parse(next) != nullptr;
	}

	template<UnaryOperator O>
	AstNode on_prefix_operator() {
		cursor_.advance();
		auto right = ast_.store(parse_subexpression(Precedence::Prefix));
		return {AstUnaryExpr {{0}, O, right}};
	}

	template<BinaryOperator O>
	AstNode on_infix_operator(AstId left) {
		auto target = cursor_.next().value();
		auto precedence = lookup_precedence_for_associativity(O);
		auto right = ast_.store(parse_subexpression(precedence));
		validate_associativity(O, left, right, target);
		return {AstBinaryExpr {{0}, O, left, right}};
	}

	template<UnaryOperator O>
	AstNode on_postfix_operator(AstId left) {
		cursor_.advance();
		return {AstUnaryExpr {{0}, O, left}};
	}

	void validate_associativity(BinaryOperator current, AstId left_id, AstId right_id, Token target) {
		auto& left_node = ast_.get(left_id);
		auto& right_node = ast_.get(right_id);

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
			auto location = target.locate_in(source_);
			auto left_str = binary_operator_to_string(left);
			auto right_str = binary_operator_to_string(right);
			report(Message::AmbiguousOperatorMixing, location, left_str, right_str);
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
			auto location = target.locate_in(source_);
			report(Message::AmbiguousOperatorMixing, location, "-", "**");
		}
	}

	AstNode on_dot(AstId left) {
		cursor_.advance();
		auto member = expect_name(Message::ExpectNameAfterDot);
		return {AstMemberExpr {{0}, left, member}};
	}

	AstNode on_infix_left_paren(AstId left) {
		const uint32_t saved = std::exchange(open_angles_, 0);
		Defer _ = [&] {
			open_angles_ = saved;
		};

		cursor_.advance();
		std::vector<AstNode> arguments;
		if (!cursor_.match(TokenKind::RightParen)) {
			do {
				arguments.emplace_back(parse_subexpression());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectClosingParen);
		}
		auto args = ast_.store_multiple(arguments);
		return {AstCallExpr {{0}, left, args}};
	}

	AstNode on_infix_left_bracket(AstId left) {
		cursor_.advance();
		auto arguments = parse_bracketed_arguments();
		return {AstIndexExpr {{0}, left, arguments}};
	}

	AstNode on_caret() {
		cursor_.advance();
		return parse_pointer_type();
	}

	AstNode on_variability() {
		cursor_.advance();
		return {parse_variability()};
	}

	AstVariabilityExpr parse_variability() {
		auto specifier = VariabilitySpecifier::Var;

		std::vector<AstNode> arguments;
		if (cursor_.match(TokenKind::LeftBrace)) {
			const uint32_t saved = std::exchange(open_angles_, 0);
			Defer _ = [&] {
				open_angles_ = saved;
			};

			specifier = VariabilitySpecifier::VarBounded;
			if (!cursor_.match(TokenKind::RightBrace)) {
				do {
					arguments.emplace_back(parse_subexpression());
				} while (cursor_.match(TokenKind::Comma));

				if (cursor_.match(TokenKind::Ellipsis)) {
					specifier = VariabilitySpecifier::VarUnbounded;
				}

				expect(TokenKind::RightBrace, Message::ExpectBraceAfterVariability);
			}
		}
		auto args = ast_.store_multiple(arguments);
		return {{0}, specifier, args};
	}

	AstNode parse_type() {
		if (cursor_.match(TokenKind::Caret)) {
			return parse_pointer_type();
		}
		if (cursor_.match(TokenKind::LeftBracket)) {
			return parse_array_type();
		}
		if (cursor_.match(TokenKind::LeftParen)) {
			return parse_function_type();
		}

		auto name = expect_name(Message::ExpectType);
		return parse_name(name);
	}

	AstNode parse_array_type() {
		OptionalAstId bound;
		if (!cursor_.match(TokenKind::RightBracket)) {
			bound = ast_.store(parse_subexpression());
			expect(TokenKind::RightBracket, Message::ExpectBracketAfterArrayBound);
		}

		auto type = ast_.store(parse_type());
		return {AstArrayTypeExpr {{0}, bound, type}};
	}

	AstNode parse_pointer_type() {
		AstVariabilityExpr variability {{0}};
		if (cursor_.match(TokenKind::Var)) {
			variability = parse_variability();
		}

		auto type = ast_.store(parse_type());
		return {AstPointerTypeExpr {{0}, variability, type}};
	}

	AstNode parse_function_type() {
		auto parameters = parse_function_type_parameters();
		expect(TokenKind::ThinArrow, Message::ExpectArrowAfterFuncTypeParams);
		auto outputs = parse_function_type_outputs();
		return {AstFunctionTypeExpr {{0}, std::move(parameters), std::move(outputs)}};
	}

	std::vector<AstFunctionParameter> parse_function_type_parameters() {
		std::vector<AstFunctionParameter> parameters;
		if (!cursor_.match(TokenKind::RightParen)) {
			do {
				parameters.emplace_back(parse_function_type_parameter());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterParams);
		}
		return parameters;
	}

	AstFunctionParameter parse_function_type_parameter() {
		auto specifier = ParameterSpecifier::None;
		if (cursor_.match(TokenKind::In)) {
			specifier = ParameterSpecifier::In;
		} else if (cursor_.match(TokenKind::Var)) {
			specifier = ParameterSpecifier::Var;
		}

		auto type = ast_.store(parse_type());

		std::string_view name;
		if (auto name_token = cursor_.match_name()) {
			name = name_token->get_lexeme(source_);
		}

		if (auto equal = cursor_.match_token(TokenKind::Equals)) {
			auto location = equal->locate_in(source_);
			report(Message::FuncTypeDefaultArgument, location);
			throw ParseError();
		}
		return {{0}, specifier, type, name};
	}

	std::vector<AstFunctionOutput> parse_function_type_outputs() {
		std::vector<AstFunctionOutput> outputs;
		if (cursor_.match(TokenKind::LeftParen)) {
			do {
				outputs.emplace_back(parse_function_type_output());
			} while (cursor_.match(TokenKind::Comma));
			expect(TokenKind::RightParen, Message::ExpectParenAfterOutputs);
		} else {
			auto type = ast_.store(parse_type());
			outputs.emplace_back(AstFunctionOutput {{0}, type, std::string_view()});
		}

		return outputs;
	}

	AstFunctionOutput parse_function_type_output() {
		auto type = ast_.store(parse_type());

		std::string_view name;
		if (auto token = cursor_.match_name()) {
			name = token->get_lexeme(source_);
		}

		return {{0}, type, name};
	}

	void rescind_lookahead(size_t saved_node_count) {
		auto first = ast_.ast_nodes_.begin() + static_cast<ptrdiff_t>(saved_node_count);
		ast_.ast_nodes_.erase(first, ast_.ast_nodes_.end());
	}

	void expect(TokenKind kind, Message message) {
		auto token = cursor_.peek();
		if (token.header.kind == kind) {
			cursor_.advance();
		} else {
			report_expectation(message, token);
			throw ParseError();
		}
	}

	std::string_view expect_name(Message message) {
		auto token = cursor_.peek();
		if (token.header.kind == TokenKind::Name) {
			cursor_.advance();
			return token.get_lexeme(source_);
		}

		report_expectation(message, token);
		return {};
	}

	void report_expectation(Message message, Token unexpected) {
		auto location = unexpected.locate_in(source_);
		report(message, location, unexpected.to_message_string(source_));
	}

	template<typename... Args>
	void report(Message message, CodeLocation location, Args&&... args) {
		if (!is_looking_ahead_) {
			reporter_.report(message, location, std::forward<Args>(args)...);
		}
	}

	static Precedence lookup_precedence_for_associativity(BinaryOperator op) {
		switch (op) {
			using enum BinaryOperator;
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

Ast parse(const SourceLock& source, Reporter& reporter) {
	auto token_stream = lex(source, reporter);
	return parse(token_stream, source, reporter);
}

Ast parse(const TokenStream& token_stream, const SourceLock& source, Reporter& reporter) {
	Parser parser(token_stream, source, reporter);
	return parser.parse();
}

} // namespace cero
