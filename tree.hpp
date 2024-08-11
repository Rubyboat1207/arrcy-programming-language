#pragma once

#include <vector>
#include <optional>
#include <string>

class ASTNode {
public:
    virtual std::ostream& print() = 0;
};

struct StatementNode : public ASTNode {
    virtual std::ostream& print() = 0;
};

struct CodeBlockNode : public StatementNode {
    std::vector<StatementNode*> stmts;
    
    std::ostream& print();
};

struct AssignmentNode : public StatementNode {
    char* name;
    std::optional<ExpressionNode*> value;

    std::ostream& print();
};

// exprs
class ExpressionNode : public ASTNode {
    virtual std::ostream& print() = 0;
};

enum ExpressionOperation {
    ADD,
    SUBTRACT,
    DIVIDE,
    MULTIPLY,
    ACCESS
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