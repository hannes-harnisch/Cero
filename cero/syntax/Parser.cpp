#include "Parser.hpp"

class Parser
{
public:
	Parser(const TokenStream&, Reporter&)
	{}

	void parse()
	{}

private:
};

void parse(const TokenStream& tokens, Reporter& reporter)
{
	Parser parser(tokens, reporter);
	return parser.parse();
}
