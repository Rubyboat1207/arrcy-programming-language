#pragma once

#include <vector>
#include <optional>
#include <string>

class ASTNode {
public:
    virtual std::ostream& print() = 0;
};

class RootNode {
public:
    std::vector<ASTNode*> children;

    RootNode() {
        children = std::vector<ASTNode*>();
    }
};

enum ExpressionOperation {
    ADD,
    SUBTRACT,
    DIVIDE,
    MULTIPLY,
    ACCESS
};

// exprs
class ExpressionNode : public ASTNode {
    virtual std::ostream& print() = 0;
};

struct BinOpNode : public ExpressionNode {
    ExpressionNode* a;
    ExpressionNode* b;
    ExpressionOperation operation;

    std::ostream& print();
};

struct LiteralNumberNode : public ExpressionNode {
    double value;

    std::ostream& print();
};

struct VariableNode : public ExpressionNode {
    std::string s;

    std::ostream& print();
};

struct ArrayNode : public ExpressionNode {
    std::vector<ExpressionNode*> values;
    std::ostream& print();
};

// statements

struct DeclarationNode : public ASTNode {
    char* name;
    std::optional<ExpressionNode*> value;

    std::ostream& print();
};