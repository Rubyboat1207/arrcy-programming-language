#include "tree.hpp"
#include "preprocessor.hpp"
#include<iostream>

// #define UGLY_PRINT


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
    #ifdef UGLY_PRINT
        return os << "BinOpNode(" << *a << operation << *b << ")";
    #else
        return os<< *a << operation << *b;
    #endif
}

void BinOpNode::accept(ExpressionVisitor &visitor)
{
    visitor.visit(this);
}

std::ostream& LiteralNumberNode::print(std::ostream& os) const {
    #ifdef UGLY_PRINT
        return os << "LitNum(" << value << ")";
    #else
        return os << value;
    #endif
}

void LiteralNumberNode::accept(ExpressionVisitor &visitor)
{
    visitor.visit(this);
}

std::ostream& VariableNode::print(std::ostream& os) const {
    #ifdef UGLY_PRINT
        return os << "Var(" << s << ")";
    #else
        return os << s;
    #endif
}

void VariableNode::accept(ExpressionVisitor &visitor)
{
    visitor.visit(this);
}

std::ostream& ArrayNode::print(std::ostream& os) const {
    #ifdef UGLY_PRINT
        os << "ArrayNode[";
    #else
        os << "[";
    #endif

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

void ArrayNode::accept(ExpressionVisitor &visitor)
{
    visitor.visit(this);
}

std::ostream& AssignmentNode::print(std::ostream& os) const {
    #ifdef UGLY_PRINT
        return os << "AssignmentNode(" << name << " = " << *value << ")";
    #else
        return os << name << " = " << *value;
    #endif
    
}

CodeBlockNode* StatementNode::append(StatementNode *node)
{
    CodeBlockNode* block = new CodeBlockNode();

    block->append(this)->append(node);

    return block;
}

std::ostream& CodeBlockNode::print(std::ostream& os) const {
    #ifdef UGLY_PRINT
        std::cout << "begin" << std::endl;
        std::cout << "statement size " << stmts.size() << std::endl;
    #endif

    for(auto stmt : stmts) {
        os << *stmt << ";" << std::endl;
    }

    #ifdef UGLY_PRINT
        std::cout << "end" << std::endl;
    #endif
    return os;
}

CodeBlockNode* CodeBlockNode::append(StatementNode *node)
{
    if(node == nullptr) {
        return this;
    }
    // check if node is already in the list
    // TODO: fix node duplication issue in parser
    // eventually, this should be removed
    // do in reverse for efficiency
    for(int i = stmts.size() - 1; i >= 0; i--) {
        auto stmt = stmts[i];
        if(stmt == node) {
            return this;
        }
    }
    stmts.push_back(node);

    return this;
}

std::ostream &ExpressionFunctionNode::print(std::ostream &os) const
{
    #ifdef UGLY_PRINT
        os << "ExpressionFunction((" << *array << ")" << type << *internal_variable;
    #else
        os << "(" << *array << ")" << type << *internal_variable;
    #endif

    if(index_variable != nullptr) {
        os << ", " << *index_variable;
    }

    #ifdef UGLY_PRINT
        return os << "{" << *action << "})";
    #else
        return os << "{" << *action << "}";
    #endif
}

void ExpressionFunctionNode::accept(ExpressionVisitor & visitor)
{
    visitor.visit(this);
}

std::ostream &StatementFunctionNode::print(std::ostream &os) const
{
    #ifdef UGLY_PRINT
        os << "StatementFunction((" << *array << ")" << *internal_variable;
    #else
        os << "(" << *array << ")" << *internal_variable;
    #endif

    if(index_variable != nullptr) {
        os << ", " << *index_variable;
    }

    #ifdef UGLY_PRINT
        return os << "{" << *action << "})";
    #else
        return os << "{\n" << *action << "}";
    #endif
}

FunctionCallNodeExpression* FunctionCallData::createNodeExpression()
{
    return new FunctionCallNodeExpression(function, parameters->copy());
}

FunctionCallNodeStatement* FunctionCallData::createNodeStatement()
{
    return new FunctionCallNodeStatement(function, parameters->copy());
}

std::ostream &FunctionCallNodeExpression::print(std::ostream &os) const
{
    os << function << "(";

    if(parameters != nullptr) {
        int i = 0;
        int size = parameters->expressions.size();
        for(int i = 0; i < size; i++) {
            auto element = parameters->expressions[i];
            if(i != size - 1) {
                os << *element << ",";
            }else {
                os << *element;
            }
        }
    }

    return os << ")";
}

void FunctionCallNodeExpression::accept(ExpressionVisitor &visitor)
{
    visitor.visit(this);
}

std::ostream& FunctionCallNodeStatement::print(std::ostream &os) const {
    os << function << "(";

    if(parameters != nullptr) {
        int i = 0;
        int size = parameters->expressions.size();
        for(int i = 0; i < size; i++) {
            auto element = parameters->expressions[i];
            if(i != size - 1) {
                os << *element << ",";
            }else {
                os << *element;
            }
        }
    }

    return os << ")";
}