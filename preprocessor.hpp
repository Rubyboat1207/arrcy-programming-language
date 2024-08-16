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
    ExpressionNode* initial_value;
    bool is_ever_read = false;
    int array_depth = 0;
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

    TypeLocatingVisitor(VariableContext* variables, PreprocessResult* result) : variables(variables), result(result) {}
};

struct FunctionTypeData {
    std::vector<VariableType> parameters;
    VariableType returnType;
};

namespace preprocessor {
    void addToFunctionData(std::string name, FunctionTypeData data);
    PreprocessResult preprocess(StatementNode* root);
    PreprocessResult processFunctionData(VariableContext* variables, FunctionCallData* node);
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