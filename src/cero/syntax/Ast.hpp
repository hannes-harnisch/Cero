#pragma once

#include "cero/io/Source.hpp"
#include "cero/syntax/AstNode.hpp"
#include "cero/syntax/AstVisitor.hpp"

#include <span>
#include <string>
#include <vector>

namespace cero {

class Ast {
public:
	uint32_t num_nodes() const;

	bool has_errors() const;

	std::span<const AstNode> raw() const;

	std::string to_string(const LockedSource& source) const;

private:
	std::vector<AstNode> nodes_;
	bool has_errors_;

	explicit Ast(std::vector<AstNode>&& nodes);

	friend class Parser;
};

} // namespace cero
