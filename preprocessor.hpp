#pragma once
#include "tree.hpp"
#include <map>


enum class PreprocessorMessageType {
    ERROR,
    WARNING,
    INFO
};

struct PreprocessorMessage {
    std::string message;
    PreprocessorMessageType type;

    PreprocessorMessage(std::string message, PreprocessorMessageType type) : message(message), type(type) {}
    friend std::ostream& operator<<(std::ostream& os, const PreprocessorMessage& obj);
};

enum class VariableType {
    NUMBER,
    ARRAY,
    MATRIX,
    VOID,
    ANY
};





struct VariableInformation {
    VariableType type;
    AssignmentNode* first_assignment;
    ExpressionNode* most_recent_assignment_expr;
    bool is_ever_read = false;
    int array_depth = 0;
    int opt_array_sizes[8] = { 0,0,0,0,0,0,0,0 };
    bool uses_array_size = false;

    VariableInformation(VariableInformation* other) {
        type = other->type;
        first_assignment = other->first_assignment;
        is_ever_read = other->is_ever_read;
        array_depth = other->array_depth;
    }

    VariableInformation(VariableType type, AssignmentNode* first_assignment) : type(type), first_assignment(first_assignment) {}
    VariableInformation() {}
};

using VariableContext = std::map<std::string, VariableInformation*>;

struct PreprocessResult {
    std::vector<PreprocessorMessage> messages;
    std::optional<VariableContext*> opt_variables;
};

class ExpressionVisitor {
public:
    virtual void visit(LiteralNumberNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(ArrayNode* node) = 0;
    virtual void visit(BinOpNode* node) = 0;
    virtual void visit(ExpressionFunctionNode* node) = 0;
    virtual void visit(FunctionCallNodeExpression* node) = 0;
};

struct DefaultExpressionVisitor : ExpressionVisitor {
    virtual void visit(LiteralNumberNode* node) {};
    virtual void visit(VariableNode* node) {};
    virtual void visit(ArrayNode* node) {
        for(auto value : node->values->expressions) {
            value->accept(*this);
        }
    };
    virtual void visit(BinOpNode* node) {
        node->a->accept(*this);

        node->b->accept(*this);
    };
    virtual void visit(ExpressionFunctionNode* node) {
        node->array->accept(*this);
        node->internal_variable->accept(*this);
        node->action->accept(*this);
    };
    virtual void visit(FunctionCallNodeExpression* node) {};
};

struct TypeLocatingVisitor : ExpressionVisitor {
    VariableContext* variables;
    VariableType ret_value;
    PreprocessResult* result;
    bool errored = false;
    int array_depth = 0;
    
    void visit(LiteralNumberNode* node) override;
    void visit(VariableNode* node) override;
    void visit(ArrayNode* node) override;
    void visit(BinOpNode* node) override;
    void visit(ExpressionFunctionNode* node) override;
    void visit(FunctionCallNodeExpression* node) override;
    
    TypeLocatingVisitor makeChild();

    TypeLocatingVisitor(VariableContext* variables, PreprocessResult* result) : variables(variables), result(result) {}
};

struct ArraySizeVisitor : DefaultExpressionVisitor {
    int depth = 0;
    int sizes[8] = { 0,0,0,0,0,0,0,0 };
    ExpressionNode* first_element;
    VariableContext* variables;
    void visit(ArrayNode* node) override;
    void visit(BinOpNode *node) override;
    void visit(VariableNode *node) override;
    void visit(ExpressionFunctionNode *node) override;
};

struct FunctionTypeData {
    std::vector<VariableType> parameters;
    VariableType returnType;
};

namespace preprocessor {
    void addToFunctionData(std::string name, FunctionTypeData data);
    PreprocessResult preprocess(StatementNode* root);
    PreprocessResult preprocess_with_ctx(StatementNode* root, VariableContext* existing_context);
    PreprocessResult processFunctionData(VariableContext* variables, FunctionCallData* node);
    std::map<std::string, FunctionTypeData> getFunctions();
}



// struct ArraySizeVisitor : ExpressionVisitor {
//     VariableContext* variables;
//     int size;
    
//     void visit(LiteralNumberNode* node) override;
//     void visit(VariableNode* node) override;
//     void visit(ArrayNode* node) override;
//     void visit(BinOpNode* node) override;
//     void visit(ExpressionFunctionNode* node) override;
// };

// VariableType get_variable_type(ExpressionNode* node);

// bool is_level_matrix_like(ArrayNode* node);