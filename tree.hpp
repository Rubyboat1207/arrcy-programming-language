#ifndef AST_TREE
#define AST_TREE
#include <vector>
#include <optional>
#include <string>
#include <iostream>
#include <cstdint>

class ExpressionVisitor;
class NodeVisitor;

class ASTNode {
public:
    virtual std::ostream& print(std::ostream& os) const = 0;
    virtual void accept(NodeVisitor& visitor) = 0;
    friend std::ostream& operator<<(std::ostream& os, const ASTNode& obj);
};

// Abstract Nodes

struct ExpressionNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;

    virtual void accept(ExpressionVisitor& visitor) = 0;
    virtual void accept(NodeVisitor& visitor) = 0;
};

struct CodeBlockNode;

struct StatementNode : public ASTNode {
    virtual std::ostream& print(std::ostream& os) const override = 0;
    
    virtual CodeBlockNode* append(StatementNode* node);
    virtual void accept(NodeVisitor& visitor) = 0;
};

// stmts

struct CodeBlockNode : public StatementNode {
    std::vector<StatementNode*> stmts;
    
    std::ostream& print(std::ostream& os) const override;

    CodeBlockNode* append(StatementNode* node);
    void accept(NodeVisitor& visitor) override;
};

struct AssignmentNode : public StatementNode {
    std::string name;
    ExpressionNode* value;

    std::ostream& print(std::ostream& os) const override;

    AssignmentNode(std::string name, ExpressionNode* value) : name(name), value(value) {}
    void accept(NodeVisitor& visitor) override;
    
};
// exprs

#define ALGEBRAIC_OPS 0b10000000
#define BOOLEAN_OPS 0b01000000
#define ARRAY_OPS 0b00100000

enum class ExpressionOperation : uint8_t {
    // algebreic ops (last bit set)
    ADD      = 0b10000000,
    SUBTRACT = 0b10000001,
    DIVIDE   = 0b10000010,
    MULTIPLY = 0b10000011,

    // boolean ops (second to last bit set)
    LT       = 0b01000000,
    GT       = 0b01000001,
    LTEQ     = 0b01000010,
    GTEQ     = 0b01000011,
    EQ       = 0b01000100,
    INEQ     = 0b01000101,

    // array operations (third to last bit set)
    ACCESS   = 0b00100000
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
    void accept(ExpressionVisitor& visitor) override;
    void accept(NodeVisitor& visitor) override;
};

struct ArrayElements {
    std::vector<ExpressionNode*> expressions;


    ArrayElements* copy() {
        auto elem = new ArrayElements();
        elem->expressions = std::vector<ExpressionNode*>(expressions);

        return elem;
    }
    ~ArrayElements() = default;
};

struct LiteralNumberNode : public ExpressionNode {
    double value;

    LiteralNumberNode(double value) : value(value) {}

    std::ostream& print(std::ostream& os) const override;
    void accept(ExpressionVisitor& visitor) override;
    void accept(NodeVisitor& visitor) override;
};

struct FunctionCallNodeExpression;
struct FunctionCallNodeStatement;

struct FunctionCallData {
    std::string function;
    ArrayElements* parameters;

    FunctionCallData(std::string function, ArrayElements* parameters) : function(function), parameters(parameters) {}

    /// @brief Creates a new Node Expression using the data from this FnCallDta
    /// this object should be deleted after, and is useless.
    /// @return The new express
    FunctionCallNodeExpression* createNodeExpression();
    /// @brief Creates a new Node Statement using the data from this FnCallDta
    /// this object should be deleted after, and is useless.
    /// @return The new express
    FunctionCallNodeStatement* createNodeStatement();

    ~FunctionCallData() {
        delete parameters;
    }
};

struct FunctionCallNodeExpression : public ExpressionNode, public FunctionCallData {
    std::ostream& print(std::ostream& os) const override;
    void accept(ExpressionVisitor& visitor) override;
    void accept(NodeVisitor& visitor) override;

    FunctionCallNodeExpression(std::string function, ArrayElements* elements) : FunctionCallData(function, elements) {}
};

struct FunctionCallNodeStatement : public StatementNode, public FunctionCallData {
    std::ostream& print(std::ostream& os) const override;
    void accept(NodeVisitor& visitor) override;

    FunctionCallNodeStatement(std::string function, ArrayElements* elements) : FunctionCallData(function, elements) {}
};

struct VariableNode : public ExpressionNode {
    std::string s;

    
    std::ostream& print(std::ostream& os) const override;
    void accept(NodeVisitor& visitor) override;

    VariableNode(std::string s) : s(s) {}
    void accept(ExpressionVisitor& visitor) override;
};

struct ArrayNode : public ExpressionNode {
    ArrayElements* values = nullptr;
    std::ostream& print(std::ostream& os) const override;
    void accept(NodeVisitor& visitor) override;

    ArrayNode(ArrayElements* values) : values(values) {}
    void accept(ExpressionVisitor& visitor) override;
    
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
    void accept(ExpressionVisitor& visitor) override;
    void accept(NodeVisitor& visitor) override;

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
    void accept(NodeVisitor& visitor) override;

    StatementFunctionNode(ExpressionNode* array, 
        VariableNode* internal_variable, VariableNode* index_variable, 
        StatementNode* action) : array(array), 
        internal_variable(internal_variable), action(action), 
        index_variable(index_variable) {}
};

class NodeVisitor {
public:
    virtual void visit(LiteralNumberNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(ArrayNode* node) = 0;
    virtual void visit(BinOpNode* node) = 0;
    virtual void visit(ExpressionFunctionNode* node) = 0;
    virtual void visit(FunctionCallNodeExpression* node) = 0;
    virtual void visit(FunctionCallNodeStatement* node) = 0;
    virtual void visit(StatementFunctionNode* node) = 0;
    virtual void visit(AssignmentNode* node) = 0;
    virtual void visit(CodeBlockNode* node) = 0;
};

#endif