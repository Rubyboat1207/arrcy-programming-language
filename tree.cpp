#include "tree.hpp"
#include<iostream>


std::ostream& operator<<(std::ostream& os, ExpressionOperation op) {
    switch (op) {
        case ExpressionOperation::ADD:       return os << "+";
        case ExpressionOperation::SUBTRACT:  return os << "-";
        case ExpressionOperation::DIVIDE:    return os << "/";
        case ExpressionOperation::MULTIPLY:  return os << "*";
        case ExpressionOperation::LT:        return os << "<";      // Less than
        case ExpressionOperation::GT:        return os << ">";      // Greater than
        case ExpressionOperation::LTEQ:      return os << "<=";    // Less than or equal to
        case ExpressionOperation::GTEQ:      return os << ">=";    // Greater than or equal to
        case ExpressionOperation::EQ:        return os << "==";      // Equal to
        case ExpressionOperation::INEQ:      return os << "!=";    // Not equal to
        case ExpressionOperation::ACCESS:    return os << "[]";  // Access operation
        default:                             return os << "???";
    }
}

std::ostream& operator<<(std::ostream& os, ExpressionFunctionType op) {
    switch (op) {
        case ExpressionFunctionType::FILTER: return os << "/";
        case ExpressionFunctionType::MAP:    return os << "#";
        case ExpressionFunctionType::REDUCE: return os << "<";
        default:                             return os << "???";
    }
}

std::ostream& operator<<(std::ostream& os, const ASTNode& obj) {
    return obj.print(os);
}

std::ostream& BinOpNode::print(std::ostream& os) const {
    return os << *a << operation << *b;
}

std::ostream& LiteralNumberNode::print(std::ostream& os) const {
    return os << value;
}

std::ostream& VariableNode::print(std::ostream& os) const {
    return os << s;
}

std::ostream& ArrayNode::print(std::ostream& os) const {
    os << "[";

    if(values != nullptr) {
        int i = 0;
        int size = values->expressions.size();
        for(int i = 0; i < size; i++) {
            auto element = values->expressions[i];
            if(i != size - 1) {
                os << *element << ",";
            }else {
                os << *element;
            }
        }
    }

    return os << "]";
}

std::ostream& AssignmentNode::print(std::ostream& os) const {
    return os << name << " = " << *value;
}

CodeBlockNode* AssignmentNode::append(StatementNode *node)
{
    CodeBlockNode* block = new CodeBlockNode();

    block->stmts.push_back(this);
    block->stmts.push_back(node);

    return block;
}

std::ostream& CodeBlockNode::print(std::ostream& os) const {
    for(auto stmt : stmts) {
        os << *stmt << ";" << std::endl;
    }

    return os;
}

CodeBlockNode* CodeBlockNode::append(StatementNode *node)
{
    stmts.push_back(node);

    return this;
}

std::ostream &ExpressionFunctionNode::print(std::ostream &os) const
{
    os << "(" << *array << ")" << type << *internal_variable;

    if(index_variable != nullptr) {
        os << ", " << *index_variable;
    }

    return os << "{" << *action << "}";
}
