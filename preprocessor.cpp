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

PreprocesResult preprocess(StatementNode *root)
{
    PreprocesResult result = PreprocesResult();

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

                if(defined_type != assigned_type) {
                    result.messages.push_back(PreprocessorMessage("Variable " + assignment->name + " is being assigned a value of a different type.", PreprocessorMessageType::ERROR));
                }
            }
            
            
        }
    }

    return result;
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
