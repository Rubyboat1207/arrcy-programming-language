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
    MATRIX
};

struct PreprocesResult {
    std::vector<PreprocessorMessage> messages;
};

struct VariableInformation {
    VariableType type;
    ExpressionNode* initial_value;
    bool is_ever_read = false;
};

using VariableContext = std::map<std::string, VariableInformation*>;


class ExpressionVisitor {
public:
    virtual void visit(LiteralNumberNode* node) = 0;
    virtual void visit(VariableNode* node) = 0;
    virtual void visit(ArrayNode* node) = 0;
    virtual void visit(BinOpNode* node) = 0;
    virtual void visit(ExpressionFunctionNode* node) = 0;
};

struct TypeLocatingVisitor : ExpressionVisitor {
    VariableContext* variables;
    VariableType ret_value;
    PreprocesResult* result;
    bool errored = false;
    
    void visit(LiteralNumberNode* node) override;
    void visit(VariableNode* node) override;
    void visit(ArrayNode* node) override;
    void visit(BinOpNode* node) override;
    void visit(ExpressionFunctionNode* node) override;

    TypeLocatingVisitor(VariableContext* variables, PreprocesResult* result) : variables(variables), result(result) {}
};

PreprocesResult preprocess(StatementNode* root);

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