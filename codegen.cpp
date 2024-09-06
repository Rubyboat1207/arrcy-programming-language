#include "codegen.hpp"
#include <string>
#include <sstream>
#include <math.h>

std::string CPPCodeGenerator::generate_variable_declaration(std::string name, VariableInformation* variable_info)
{
    std::string type_str = "double";
    std::string array = "";
    for(int i = 0; i < variable_info->array_depth; i++) {
        array += "[]";
    }

    std::string equality;

    if(variable_info->first_assignment != nullptr) {
        auto visitor = CPPExpressionGenerator();
        variable_info->first_assignment->value->accept(visitor);

        equality = " = " + visitor.expr;
    }

    variable_info->most_recent_assignment_expr = variable_info->first_assignment->value;
    
    return type_str + " " + name + array + equality;
}

std::string CPPCodeGenerator::generate_variable_assignment(std::string var, ExpressionNode* expr)
{
    if(global_context->find(var) != global_context->end()) {
        global_context->at(var)->most_recent_assignment_expr = expr;
    }
    auto visitor = CPPExpressionGenerator();
    expr->accept(visitor);
    std::string str = var + "=" + visitor.expr;

    return str;
}

void CPPCodeGenerator::generate_foreach_loop(StatementFunctionNode* node)
{
    CPPExpressionGenerator visitor;
    node->array->accept(visitor);

    if(node->index_variable != nullptr) {

        int size = 0;

        ArraySizeVisitor size_visitor{};

        size_visitor.variables = global_context;

        node->array->accept(size_visitor);

        str += "for(int " + node->index_variable->s + " = 0; " + node->index_variable->s + " < " + std::to_string(size_visitor.size) + "; " + node->index_variable->s + "++) {";
        str += "\n#define " + node->internal_variable->s + " " + visitor.expr + "[" + node->index_variable->s + "]" + "\n";
        node->action->accept(*this);
        CodeBlockNode* action_block = dynamic_cast<CodeBlockNode*>(node->action);
        if(action_block == nullptr) {
            str += ";";
        }
        str += "\n#undef " + node->internal_variable->s + "\n";
    }else {
        str += "for(auto " + node->internal_variable->s + " : " + visitor.expr + ") {";
        node->action->accept(*this);
        CodeBlockNode* action_block = dynamic_cast<CodeBlockNode*>(node->action);
        if(action_block == nullptr) {
            str += ";";
        }
    }
    
    

    

    str += "}";
}

std::string CPPCodeGenerator::generate_compiler_foreach(CodeBlockNode* internal, std::string index_name, int index_count)
{
    std::string code = "for(int " + index_name + " = 0; " + index_name + " < " + std::to_string(index_count) + "; " + index_name + "++) {";
    for(StatementNode* node : internal->stmts) {
        node->accept(*this);
        code += ";\n";
    }

    return code;
}

std::string CPPCodeGenerator::generate_print_statement(ArrayElements* callData)
{
    std::string code = "std::cout <<";

    for(ExpressionNode* expr : callData->expressions) {
        auto visitor = CPPExpressionGenerator();
        expr->accept(visitor);

        code += visitor.expr + "<<";
    }

    code += "std::endl";

    return code;
}

std::string CPPCodeGenerator::generate_element_assignment(ElementAssignmentNode *node)
{
    auto indexVisitor = CPPExpressionGenerator();
    node->index->accept(indexVisitor);

    auto valueVisitor = CPPExpressionGenerator();
    node->value->accept(valueVisitor);

    std::string str = node->name + "[" + indexVisitor.expr + "] " + "=" + valueVisitor.expr;

    return str;
}

void CPPExpressionGenerator::visit(LiteralNumberNode *node)
{
    if(node->value == std::floor(node->value)) {
        expr += std::to_string((int) node->value);
    }else {
        expr += std::to_string(node->value);
    }
}

void CPPExpressionGenerator::visit(VariableNode *node)
{
    expr += node->s;
}

void CPPExpressionGenerator::visit(ArrayNode *node)
{
    expr += "{";

    if(node->values != nullptr) {
        int i = 0;
        int size = node->values->expressions.size();
        for(int i = 0; i < size; i++) {
            auto element = node->values->expressions[i];
            element->accept(*this);
            
            if(i != size - 1) {
                expr += ",";
            }
        }
    }

    expr += "}";
}

void CPPExpressionGenerator::visit(BinOpNode *node)
{
    node->a->accept(*this);
    switch (node->operation) {
        case ExpressionOperation::ADD:       expr += "+"; break;
        case ExpressionOperation::SUBTRACT:  expr += "-"; break;
        case ExpressionOperation::DIVIDE:    expr += "/"; break;
        case ExpressionOperation::MULTIPLY:  expr += "*"; break;
        case ExpressionOperation::LT:        expr += "<"; break;      // Less than
        case ExpressionOperation::GT:        expr += ">"; break;      // Greater than
        case ExpressionOperation::LTEQ:      expr += "<="; break;    // Less than or equal to
        case ExpressionOperation::GTEQ:      expr += ">="; break;    // Greater than or equal to
        case ExpressionOperation::EQ:        expr += "=="; break;      // Equal to
        case ExpressionOperation::INEQ:      expr += "!="; break;    // Not equal to
        case ExpressionOperation::ACCESS:    expr += "["; break;  // Access operation
        default:                             expr += "???"; break;
    }
    node->b->accept(*this);
    if(node->operation == ExpressionOperation::ACCESS) {
        expr += "]";
    }
}

void CPPExpressionGenerator::visit(ExpressionFunctionNode *node)
{
    expr += "/* TODO */";
}

void CPPExpressionGenerator::visit(FunctionCallNodeExpression *node)
{
    expr += node->function + "(";
    
    for(auto entry : node->parameters->expressions) {
        entry->accept(*this);
        if(entry != node->parameters->expressions[node->parameters->expressions.size() - 1]) {
            expr += ",";
        }
    }

    expr += ")";
}

std::string CodeGenerator::generate(StatementNode* rootNode, PreprocessResult result)
{
    global_context = nullptr;

    if(result.opt_variables.has_value()) {
        global_context = result.opt_variables.value();
    }

    str += generate_prefix();
    // if(result.opt_variables.has_value()) {
    //     for(const auto & pair : (*result.opt_variables.value())) {
    //         str += generate_variable_declaration(pair.first, pair.second) + ";\n";
    //     }
    // }
    rootNode->accept(*this);
    str += generate_suffix();
    return str;
}

void CodeGenerator::visit(CodeBlockNode *node)
{
    for(StatementNode* node : node->stmts) {
        node->accept(*this);
        str += ";\n";
    }
}

void CodeGenerator::visit(AssignmentNode *node)
{
    if(global_context->find(node->name) != global_context->end()) {
        auto variable_info = global_context->at(node->name);
        
        if(variable_info->first_assignment == node) {
            str += generate_variable_declaration(node->name, variable_info);
            return;
        }
        // else {
        //     if(variable_info->type == VariableType::ARRAY) {
        //         CodeBlockNode* internal = new CodeBlockNode();
        //         internal->stmts.push_back(node);
        //         int size = ((ArrayNode*) variable_info->first_assignment->value)->values->expressions.size();
        //         str += generate_compiler_foreach(internal, generate_safe_variable_name(), size);
        //     }
        // }

        
    }
    str += generate_variable_assignment(node->name, node->value);
}

void CodeGenerator::visit(FunctionCallNodeStatement *node)
{
    str += generate_function(node);
}

void CodeGenerator::visit(ElementAssignmentNode *node)
{
    str += generate_element_assignment(node);
}

void CodeGenerator::visit(StatementFunctionNode *node)
{
    generate_foreach_loop(node);
}

std::string CodeGenerator::generate_function(FunctionCallNodeStatement* callData)
{
    if(callData->function == "print") {
        return generate_print_statement(callData->parameters);
    }

    return "";
}
