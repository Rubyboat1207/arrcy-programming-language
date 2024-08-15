#include "preprocessor.hpp"

std::ostream &operator<<(std::ostream &os, const PreprocessorMessage &obj)
{
    switch (obj.type) {
        case PreprocessorMessageType::ERROR:
            return os << "\033[1;31mERROR: " << obj.message << "\033[0m";
        case PreprocessorMessageType::WARNING:
            return os << "\033[1;33mWARN: " << obj.message << "\033[0m";
        case PreprocessorMessageType::INFO:
            return os << "\033[1;34mINFO: " << obj.message << "\033[0m";
    }
    return os;
}

namespace preprocessor {
std::map<std::string, FunctionTypeData> functionData = {
    {"min", FunctionTypeData{{VariableType::NUMBER, VariableType::NUMBER}, VariableType::NUMBER}},
    {"max", FunctionTypeData{{VariableType::NUMBER, VariableType::NUMBER}, VariableType::NUMBER}},
    {"print", FunctionTypeData{{VariableType::ANY}, VariableType::VOID}},
};

void addToFunctionData(std::string name, FunctionTypeData data)
{
    functionData[name] = data;
}

PreprocessResult preprocess(StatementNode *root)
{
    PreprocessResult result = PreprocessResult();

    CodeBlockNode* code_block = dynamic_cast<CodeBlockNode*>(root);

    if(code_block == nullptr) {
        // its one line, so its not worth preprocessing
        return result;
    }

    VariableContext variables = {};

    for(auto stmt : code_block->stmts) {
        AssignmentNode* assignment = dynamic_cast<AssignmentNode*>(stmt);
        if(assignment != nullptr) {
            auto type_visitor = TypeLocatingVisitor(&variables, &result);
            assignment->value->accept(type_visitor);

            if(type_visitor.errored) {
                return result;
            }

            auto assigned_type = type_visitor.ret_value;
            if(variables[assignment->name] == nullptr) {
                VariableInformation* info = new VariableInformation();
                info->type = assigned_type;
                variables[assignment->name] = info;
            }else {
                auto defined_type = variables[assignment->name]->type;

                if(defined_type != VariableType::ANY && assigned_type != VariableType::ANY) {
                    if(defined_type != assigned_type) {
                        result.messages.push_back(PreprocessorMessage("Variable " + assignment->name + " is being assigned a value of a different type.", PreprocessorMessageType::ERROR));
                    }
                }
            }
        }
        FunctionCallNodeStatement* fnCallStmt = dynamic_cast<FunctionCallNodeStatement*>(stmt);
        if(fnCallStmt != nullptr) {
            auto msgs = processFunctionData(&variables, fnCallStmt).messages;
            result.messages.insert(result.messages.end(), msgs.begin(), msgs.end());
        }
    }

    return result;
}
PreprocessResult processFunctionData(VariableContext *variables, FunctionCallData *node)
{
    PreprocessResult result = PreprocessResult();
    auto fn = preprocessor::functionData.find(node->function);

    if(fn != preprocessor::functionData.end()) {
        // might as well check if params are correct, while we're here.
        FunctionTypeData fcd = (*fn).second;

        int given_param_size = 0;
        if(node->parameters != nullptr) {
            given_param_size = node->parameters->expressions.size();
        }
        
        int expected_param_size = fcd.parameters.size();
        if(given_param_size != expected_param_size) {
            result.messages.push_back(PreprocessorMessage(
                "Parameter count mismatch, expected " + std::to_string(expected_param_size) + " found " + std::to_string(given_param_size),
                PreprocessorMessageType::ERROR
            ));
            return result;
        }
        auto type_visitor = TypeLocatingVisitor(variables, &result);
        for(int i = 0; i < expected_param_size; i++) {
            VariableType expected_type = fcd.parameters[i];
            if(expected_type == VariableType::ANY) {
                continue;
            }
            ExpressionNode* expr = node->parameters->expressions[i];
            expr->accept(type_visitor);

            if(expected_type != type_visitor.ret_value) {
                result.messages.push_back(PreprocessorMessage(
                    "Parameter " + std::to_string(i + 1) + " is not the correct type on function " + (*fn).first,
                    PreprocessorMessageType::ERROR
                ));
                return result;
            }
        }
    }else {
        result.messages.push_back(PreprocessorMessage("Function " + node->function + " is not defined.", PreprocessorMessageType::ERROR));
    }
    return result;
}
}

void TypeLocatingVisitor::visit(LiteralNumberNode *node)
{
    ret_value = VariableType::NUMBER;
}

void TypeLocatingVisitor::visit(VariableNode *node)
{
    auto definedVariable = (*variables)[node->s];

    if(definedVariable == nullptr) {
        errored = true;
        result->messages.push_back(PreprocessorMessage("Variable " + node->s + " was used before it was defined.", PreprocessorMessageType::ERROR));
        return;
    }

    ret_value = definedVariable->type;
}

void TypeLocatingVisitor::visit(ArrayNode *node)
{
    ret_value = VariableType::ARRAY;
}

void TypeLocatingVisitor::visit(BinOpNode *node)
{
    VariableType left;
    VariableType right;

    node->a->accept(*this);
    left = ret_value;

    node->b->accept(*this);
    right = ret_value;

    if(left == VariableType::NUMBER && right == VariableType::NUMBER) {
        ret_value = VariableType::NUMBER;
    } else {
        ret_value = VariableType::ARRAY;
    }
}

void TypeLocatingVisitor::visit(ExpressionFunctionNode *node)
{
    if(node->type == ExpressionFunctionType::REDUCE) {
        ret_value = VariableType::NUMBER;
        return;
    }
    ret_value = VariableType::ARRAY;
}

void TypeLocatingVisitor::visit(FunctionCallNodeExpression *node)
{
    auto msgs = preprocessor::processFunctionData(variables, node).messages;
    result->messages.insert(result->messages.end(), msgs.begin(), msgs.end());

    if(msgs.size() > 0) {
        errored = true;
        return;
    }

    auto fn = preprocessor::functionData.find(node->function);

    if(fn != preprocessor::functionData.end()) {
        if((*fn).second.returnType == VariableType::VOID) {
            result->messages.push_back(PreprocessorMessage("Function " + node->function + " is not allowed in an expression.", PreprocessorMessageType::ERROR));
            errored = true;
            return;
        }
        ret_value = (*fn).second.returnType;
    }else {
        errored = true;
    }
}

// void ArraySizeVisitor::visit(LiteralNumberNode *node)
// {
//     return;
// }

// void ArraySizeVisitor::visit(VariableNode *node)
// {
//     auto definedVariable = (*variables)[node->s];

//     definedVariable->initial_value->accept(*this);
// }

// void ArraySizeVisitor::visit(ArrayNode *node)
// {
//     size = node->values->expressions.size();
// }

// void ArraySizeVisitor::visit(BinOpNode *node)
// {
//     int left;
//     int right;

//     node->a->accept(*this);
//     left = size;

//     node->b->accept(*this);
//     right = size;

//     if(left != right) {
        
//     }
// }

// void ArraySizeVisitor::visit(ExpressionFunctionNode *node)
// {
//     if(node->type == ExpressionFunctionType::REDUCE) {
//         ret_value = VariableType::NUMBER;
//         return;
//     }
//     ret_value = VariableType::ARRAY;
// }




// // an array is a matrix if the following two conditions are met:
// // 1. all elements are arrays except for leafs
// // 2. all arrays have the same length
// bool is_level_matrix_like(ArrayNode *node)
// {
//     bool is_leaf = false;
//     int size = -1;
//     for(auto expr : node->values->expressions) {
//         ArrayNode* array = dynamic_cast<ArrayNode*>(expr);

//         if(array == nullptr) {
//             is_leaf = true;
//             break;
//         }else if(is_leaf) { // found an array in the same level as a leaf
//             return false;
//         }

//         if(size == -1) {
//             size = array->values->expressions.size();
//         } else if(size != array->values->expressions.size()) {
//             return false;
//         }
//     }

//     if(!is_leaf) {
//         return true;
//     }
// }
