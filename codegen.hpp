#include"preprocessor.hpp"
#include"tree.hpp"

struct CodeGenerator : public DefaultNodeVisitor {
    std::string str;


    std::string generate(StatementNode* rootNode, PreprocessResult result);
    
    void visit(CodeBlockNode* node) override;
    void visit(AssignmentNode* node) override;
    void visit(FunctionCallNodeStatement* node) override;
protected:
    virtual std::string generate_variable_declaration(std::string name, VariableInformation* variable_info) = 0;
    virtual std::string generate_variable_assignment(std::string var, ExpressionNode* expr) = 0;
    virtual std::string generate_prefix() = 0;
    virtual std::string generate_suffix() = 0;

    virtual std::string generate_foreach_loop(StatementFunctionNode node, VariableInformation* variable_info, std::string name) = 0;
    std::string generate_function(FunctionCallNodeStatement* callData);

    virtual std::string generate_print_statement(ArrayElements* callData) = 0;
};

struct CPPCodeGenerator : public CodeGenerator {
protected:
    std::string generate_variable_declaration(std::string name, VariableInformation* variable_info) override;
    std::string generate_variable_assignment(std::string var, ExpressionNode* expr) override;
    std::string generate_prefix() {
        return "#include<iostream>\n\nint main() {\n";
    }
    std::string generate_suffix() {
        return "}";
    }

    std::string generate_foreach_loop(StatementFunctionNode node, VariableInformation* variable_info, std::string name) override;
    std::string generate_print_statement(ArrayElements* callData) override;
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