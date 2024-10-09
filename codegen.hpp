#include"preprocessor.hpp"
#include"tree.hpp"

struct CodeGenerator : public DefaultNodeVisitor {
    std::string str;

    VariableContext* global_context;
    std::string generate(StatementNode* rootNode, PreprocessResult result);
    
    void visit(CodeBlockNode* node) override;
    void visit(AssignmentNode* node) override;
    void visit(FunctionCallNodeStatement* node) override;
    void visit(ElementAssignmentNode* node) override;
    void visit(StatementFunctionNode* node) override;
protected:
    virtual std::string generate_variable_declaration(std::string name, VariableInformation* variable_info) = 0;
    virtual std::string generate_variable_assignment(std::string var, ExpressionNode* expr) = 0;
    virtual std::string generate_element_assignment(ElementAssignmentNode* node) = 0;
    virtual std::string generate_prefix() = 0;
    virtual std::string generate_suffix() = 0;

    virtual void generate_foreach_loop(StatementFunctionNode* node) = 0;
    virtual std::string generate_compiler_foreach(CodeBlockNode* internal, std::string index_name, int index_count) = 0;
    virtual std::string generate_safe_variable_name() = 0;
    std::string generate_function(FunctionCallNodeStatement* callData);

    virtual std::string generate_print_statement(ArrayElements* callData) = 0;
};

struct CPPCodeGenerator : public CodeGenerator {
public:
    std::string generate_variable_declaration(std::string name, VariableInformation* variable_info) override;
    std::string generate_variable_assignment(std::string var, ExpressionNode* expr) override;
    std::string generate_prefix() {
        return "#include<iostream>\n\nint main() {\n";
    }
    std::string generate_suffix() {
        return "}";
    }

    void generate_foreach_loop(StatementFunctionNode* node) override;
    std::string generate_print_statement(ArrayElements* callData) override;
    std::string generate_element_assignment(ElementAssignmentNode* node) override;
    std::string generate_compiler_foreach(CodeBlockNode* internal, std::string index_name, int index_count) override;
    std::string generate_safe_variable_name() override {
        char* buffer = new char[50];

        sprintf(buffer, "generated_%d", rand());
        
        while(global_context->find(buffer) != global_context->end()) {
            sprintf(buffer, "generated_%d", rand());
        }

        return std::string(buffer);
    }
};

struct CPPExpressionGenerator : public ExpressionVisitor {
    std::string expr;
    std::string pre_statement_setup;
    VariableContext* variables;
    CPPCodeGenerator* codegen;

    void visit(LiteralNumberNode* node) override;
    void visit(VariableNode* node) override;
    void visit(ArrayNode* node) override;
    void visit(BinOpNode* node) override;
    void visit(ExpressionFunctionNode* node) override;
    void visit(FunctionCallNodeExpression* node) override;

    CPPExpressionGenerator* make_child();
};