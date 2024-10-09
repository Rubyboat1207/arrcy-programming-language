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

    VariableContext* variables = new VariableContext();

    for(auto stmt : code_block->stmts) {
        AssignmentNode* assignment = dynamic_cast<AssignmentNode*>(stmt);
        if(assignment != nullptr) {
            auto type_visitor = TypeLocatingVisitor(variables, &result);
            assignment->value->accept(type_visitor);

            if(type_visitor.errored) {
                return result;
            }

            auto assigned_type = type_visitor.ret_value;
            if((*variables)[assignment->name] == nullptr) {
                VariableInformation* info = new VariableInformation();
                info->type = assigned_type;
                (*variables)[assignment->name] = info;
                info->array_depth = type_visitor.array_depth;
                info->first_assignment = assignment;
            }else {
                VariableInformation* info = (*variables)[assignment->name];
                auto defined_type = info->type;

                if(defined_type != VariableType::ANY && assigned_type != VariableType::ANY) {
                    if(defined_type != assigned_type) {
                        result.messages.push_back(PreprocessorMessage("Variable " + assignment->name + " is being assigned a value of a different type.", PreprocessorMessageType::ERROR));
                    }
                    int effective_depth = info->array_depth;

                    bool isElementAssignment = dynamic_cast<ElementAssignmentNode*>(assignment) != nullptr;
                    if(isElementAssignment) {
                        effective_depth -= 1;
                    }
                    if(effective_depth != type_visitor.array_depth) {
                        result.messages.push_back(PreprocessorMessage("Variable " + assignment->name + " is being assigned an array of a different depth (" + std::to_string((*variables)[assignment->name]->array_depth) + " <- " + std::to_string(effective_depth) + ").", PreprocessorMessageType::ERROR));
                    }
                }
            }
        }
        FunctionCallNodeStatement* fnCallStmt = dynamic_cast<FunctionCallNodeStatement*>(stmt);
        if(fnCallStmt != nullptr) {
            auto msgs = processFunctionData(variables, fnCallStmt).messages;
            result.messages.insert(result.messages.end(), msgs.begin(), msgs.end());
        }
    }

    result.opt_variables = variables;

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
std::map<std::string, FunctionTypeData> getFunctions()
{
    return functionData;
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
    ArraySizeVisitor size_visitor{};

    size_visitor.variables = variables;

    node->accept(size_visitor);

    array_depth = size_visitor.depth;
    
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

    ArraySizeVisitor size_visitor{};
    size_visitor.variables = variables;

    node->a->accept(size_visitor);

    if(left == VariableType::NUMBER && right == VariableType::NUMBER) {
        ret_value = VariableType::NUMBER;
    } else {
        ret_value = VariableType::ARRAY;
        array_depth = size_visitor.depth;
    }
}

void TypeLocatingVisitor::visit(ExpressionFunctionNode *node)
{
    if(node->type == ExpressionFunctionType::REDUCE) {
        ret_value = VariableType::NUMBER;
        return;
    }

    TypeLocatingVisitor child_array_validator = makeChild();

    node->array->accept(child_array_validator);

    if(child_array_validator.ret_value == VariableType::NUMBER) {
        errored = true;
        result->messages.push_back(PreprocessorMessage("Input to expression function node was not an array.", PreprocessorMessageType::ERROR));
        return;
    }

    if(child_array_validator.ret_value == VariableType::ANY) {
        result->messages.push_back(PreprocessorMessage("Input to expression function node is unknown at runtime. Be careful.", PreprocessorMessageType::WARNING));
    }
    

    ret_value = VariableType::ARRAY;
    ArraySizeVisitor size_visitor{};
    size_visitor.variables = variables;

    node->array->accept(size_visitor);
    array_depth = size_visitor.depth;
    
    TypeLocatingVisitor child = makeChild();

    VariableInformation* internal_var_type = new VariableInformation();
    VariableInformation* index_variable = nullptr;
    internal_var_type->first_assignment = nullptr;
    internal_var_type->most_recent_assignment_expr = nullptr;

    if(array_depth == 1) {
        internal_var_type->type = VariableType::NUMBER;
    }else {
        internal_var_type->type = VariableType::ARRAY;
        internal_var_type->array_depth = array_depth - 1;
    }
    
    (*child.variables)[node->internal_variable->s] = internal_var_type;

    if(node->index_variable != nullptr) {
        index_variable = new VariableInformation(VariableType::NUMBER, nullptr);
        index_variable->first_assignment = nullptr;
        index_variable->most_recent_assignment_expr = nullptr;
    }

    node->action->accept(child);

    // we love memory safety
    delete internal_var_type;

    if(index_variable != nullptr) {
        delete index_variable;
    }
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

TypeLocatingVisitor TypeLocatingVisitor::makeChild()
{
    return TypeLocatingVisitor(variables, result);
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

void ArraySizeVisitor::visit(ArrayNode *node)
{
    depth++;
    size = node->values->expressions.size();
    if(size == 0) {
        return;
    }
    node->values->expressions[0]->accept(*this);
    first_element = node->values->expressions[0];
}

void ArraySizeVisitor::visit(BinOpNode *node)
{
    //ignore
}

void ArraySizeVisitor::visit(VariableNode *node)
{
    if(variables == nullptr) {
        return;
    }
    
    if(variables->find(node->s) == variables->end()) {
        return;
    }

    auto variable = variables->at(node->s);

    if(variable->most_recent_assignment_expr == nullptr) {
        if(variable->first_assignment == nullptr) {
            return;
        }
        variable->first_assignment->value->accept(*this);
    }else {
        std::cout << *node << std::endl;
        variable->most_recent_assignment_expr->accept(*this);
    }
    depth = variable->array_depth;
}
