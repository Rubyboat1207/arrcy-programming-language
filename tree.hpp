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

// Abstract Nodes

struct ExpressionNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;
};

struct CodeBlockNode;

struct StatementNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;

    virtual CodeBlockNode* append(StatementNode* node);
};

// stmts

struct CodeBlockNode : public StatementNode {
    std::vector<StatementNode*> stmts;
    
    std::ostream& print(std::ostream& os) const override;

    CodeBlockNode* append(StatementNode* node);
};

struct AssignmentNode : public StatementNode {
    std::string name;
    ExpressionNode* value;

    std::ostream& print(std::ostream& os) const override;

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

struct ModificationAssign {
    ExpressionOperation operation;
    ExpressionNode* value;

    ModificationAssign(ExpressionOperation operation, ExpressionNode* value) : operation(operation), value(value) {}
};

struct BinOpNode : public ExpressionNode {
    ExpressionNode* a;
    ExpressionNode* b;
    ExpressionOperation operation;

    BinOpNode(ExpressionNode* a, ExpressionNode* b, ExpressionOperation operation) : a(a), b(b), operation(operation) {}

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

    VariableNode(std::string s) : s(s) {}
};

struct ArrayElements {
    std::vector<ExpressionNode*> expressions;
};

struct ArrayNode : public ExpressionNode {
    ArrayElements* values = nullptr;
    std::ostream& print(std::ostream& os) const override;

    ArrayNode(ArrayElements* values) : values(values) {}
};

enum class ExpressionFunctionType {
    MAP,
    FILTER,
    REDUCE
};
std::ostream& operator<<(std::ostream& os, ExpressionFunctionType op);

struct ExpressionFunctionNode : public ExpressionNode {
    ExpressionFunctionType type;
    VariableNode* internal_variable;
    ExpressionNode* action;
    ExpressionNode* array;
    // nullable
    VariableNode* index_variable;

    std::ostream& print(std::ostream& os) const override;

    ExpressionFunctionNode(
        ExpressionFunctionType type, ExpressionNode* array, 
        VariableNode* internal_variable, VariableNode* index_variable, 
        ExpressionNode* action) : 

        type(type), array(array), 
        internal_variable(internal_variable), action(action), 
        index_variable(index_variable) {}
};

// only ForEach right now, ill add an enum if i need more
struct StatementFunctionNode : public StatementNode {
    VariableNode* internal_variable;
    StatementNode* action;
    ExpressionNode* array;
    // nullable
    VariableNode* index_variable;

    std::ostream& print(std::ostream& os) const override;
    

    StatementFunctionNode(ExpressionNode* array, 
        VariableNode* internal_variable, VariableNode* index_variable, 
        StatementNode* action) : array(array), 
        internal_variable(internal_variable), action(action), 
        index_variable(index_variable) {}
};

#endif