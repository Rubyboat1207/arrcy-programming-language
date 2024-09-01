#include "codegen.hpp"
#include <string>
#include <sstream>

std::string CPPCodeGenerator::generate_variable_declaration(std::string name, VariableInformation* variable_info)
{
    std::string type_str = "double";
    for(int i = 0; i < variable_info->array_depth; i++) {
        type_str += "[]";
    }

    return type_str + " " + name;
}

std::string CPPCodeGenerator::generate_variable_assignment(std::string var, ExpressionNode* expr)
{
    auto visitor = CPPExpressionGenerator();
    expr->accept(visitor);
    std::string str = var + "=" + visitor.expr;

    return str;
}

std::string CPPCodeGenerator::generate_foreach_loop(StatementFunctionNode node, VariableInformation* variable_info, std::string name)
{
    auto var_info = new VariableInformation(variable_info);
    var_info->array_depth -= 1;
    return "for(" + generate_variable_declaration(node.internal_variable->s, var_info) + " : " + name + ") {";
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

void CPPExpressionGenerator::visit(LiteralNumberNode *node)
{
    expr += std::to_string(node->value);
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
        case ExpressionOperation::ACCESS:    expr += "[]"; break;  // Access operation
        default:                             expr += "???"; break;
    }
    node->b->accept(*this);
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
    str += generate_prefix();
    if(result.opt_variables.has_value()) {
        for(const auto & pair : (*result.opt_variables.value())) {
            str += generate_variable_declaration(pair.first, pair.second) + ";\n";
        }
    }
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
    str += generate_variable_assignment(node->name, node->value);
}

void CodeGenerator::visit(FunctionCallNodeStatement *node)
{
    str += generate_function(node);
}

std::string CodeGenerator::generate_function(FunctionCallNodeStatement* callData)
{
    if(callData->function == "print") {
        return generate_print_statement(callData->parameters);
    }

    return "";
}
