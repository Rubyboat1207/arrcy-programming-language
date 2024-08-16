#include "codegen.hpp"
#include <string>
#include <sstream>

std::string CPPCodeGenerator::generate_variable_declaration(VariableType type, std::string name, VariableInformation* variable_info)
{
    std::string type_str;
    switch (type) {
        case VariableType::NUMBER:
            type_str = "double";
            break;
        case VariableType::ARRAY:
            type_str = "double[]";
            break;
    }

    return type_str + " " + name + ";\n";
}

std::string CPPCodeGenerator::generate_variable_assignment(VariableNode* var, ExpressionNode* expr)
{
    auto visitor = CPPExpressionGenerator();
    expr->accept(visitor);
    std::string str = var->s + "=" + visitor.expr;
}

std::string CPPCodeGenerator::generate_foreach_loop(StatementFunctionNode node)
{
    return "for("
}

void CPPExpressionGenerator::visit(LiteralNumberNode *node)
{
    expr += node->value;
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
        case ExpressionOperation::ADD:       expr + "+"; break;
        case ExpressionOperation::SUBTRACT:  expr + "-"; break;
        case ExpressionOperation::DIVIDE:    expr + "/"; break;
        case ExpressionOperation::MULTIPLY:  expr + "*"; break;
        case ExpressionOperation::LT:        expr + "<"; break;      // Less than
        case ExpressionOperation::GT:        expr + ">"; break;      // Greater than
        case ExpressionOperation::LTEQ:      expr + "<="; break;    // Less than or equal to
        case ExpressionOperation::GTEQ:      expr + ">="; break;    // Greater than or equal to
        case ExpressionOperation::EQ:        expr + "=="; break;      // Equal to
        case ExpressionOperation::INEQ:      expr + "!="; break;    // Not equal to
        case ExpressionOperation::ACCESS:    expr + "[]"; break;  // Access operation
        default:                             expr + "???"; break;
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