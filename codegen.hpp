#include"preprocessor.hpp"
#include"tree.hpp"

struct CodeGenerator {
    virtual std::string generate_variable_declaration(VariableType type, std::string name, VariableInformation* variable_info) = 0;
    virtual std::string generate_variable_assignment(VariableNode* var, ExpressionNode* expr) = 0;

    virtual std::string generate_foreach_loop(StatementFunctionNode node) = 0;
};

struct CPPCodeGenerator : public CodeGenerator {
    std::string generate_variable_declaration(VariableType type, std::string name, VariableInformation* variable_info) override;
    std::string generate_variable_assignment(VariableNode* var, ExpressionNode* expr) override;

    std::string generate_foreach_loop(StatementFunctionNode node) override;
};

struct CPPExpressionGenerator : public ExpressionVisitor {
    std::string expr;

    void visit(LiteralNumberNode* node) override;
    void visit(VariableNode* node) override;
    void visit(ArrayNode* node) override;
    void visit(BinOpNode* node) override;
    void visit(ExpressionFunctionNode* node) override;
    void visit(FunctionCallNodeExpression* node) override;
};