#ifndef AST_TREE
#define AST_TREE
#include <vector>
#include <optional>
#include <string>
#include <iostream>

class ASTNode {
public:
    virtual std::ostream& print(std::ostream& os) const = 0;
    friend std::ostream& operator<<(std::ostream& os, const ASTNode& obj);
};

struct ExpressionNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;
};

struct CodeBlockNode;

struct StatementNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;

    virtual CodeBlockNode* append(StatementNode* node) = 0;
};

struct CodeBlockNode : public StatementNode {
    std::vector<StatementNode*> stmts;
    
    std::ostream& print(std::ostream& os) const override;

    CodeBlockNode* append(StatementNode* node);
};

struct AssignmentNode : public StatementNode {
    std::string name;
    ExpressionNode* value;

    std::ostream& print(std::ostream& os) const override;
    CodeBlockNode* append(StatementNode* node);

    AssignmentNode(std::string name, ExpressionNode* value) : name(name), value(value) {}
};

// exprs


enum class ExpressionOperation {
    // algebreic ops
    ADD,
    SUBTRACT,
    DIVIDE,
    MULTIPLY,

    // boolean ops
    LT,
    GT,
    LTEQ,
    GTEQ,
    EQ,
    INEQ,

    // well, this one...
    ACCESS
};
std::ostream& operator<<(std::ostream& os, ExpressionOperation op);


struct BinOpNode : public ExpressionNode {
    ExpressionNode* a;
    ExpressionNode* b;
    ExpressionOperation operation;

    std::ostream& print(std::ostream& os) const override;
};

struct LiteralNumberNode : public ExpressionNode {
    double value;

    LiteralNumberNode(double value) : value(value) {}

    std::ostream& print(std::ostream& os) const override;
};

struct VariableNode : public ExpressionNode {
    std::string s;

    std::ostream& print(std::ostream& os) const override;
};

struct ArrayNode : public ExpressionNode {
    std::vector<ExpressionNode*> values;
    std::ostream& print(std::ostream& os) const override;
};

#endif