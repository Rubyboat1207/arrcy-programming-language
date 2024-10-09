#include "codegen.hpp"
#include <string>
#include <sstream>
#include <math.h>

std::string CPPCodeGenerator::generate_variable_declaration(std::string name, VariableInformation* variable_info)
{
    std::string type_str = "double";
    std::string array = "";
    for(int i = 0; i < variable_info->array_depth; i++) {
        array += "*";
    }
    std::string setup = "";
    std::string equality;

    if(variable_info->first_assignment != nullptr) {
        auto visitor = CPPExpressionGenerator();
        visitor.variables = this->global_context;
        visitor.codegen = this;
        variable_info->first_assignment->value->accept(visitor);

        setup = visitor.pre_statement_setup;
        equality = " = " + visitor.expr;
    }

    variable_info->most_recent_assignment_expr = variable_info->first_assignment->value;
    
    return setup + type_str + array + " " + name + equality;
}

std::string CPPCodeGenerator::generate_variable_assignment(std::string var, ExpressionNode* expr)
{
    if(global_context->find(var) != global_context->end()) {
        global_context->at(var)->most_recent_assignment_expr = expr;
    }
    auto visitor = CPPExpressionGenerator();
    visitor.variables = this->global_context;
    visitor.codegen = this;
    expr->accept(visitor);
    std::string str = visitor.pre_statement_setup + var + "=" + visitor.expr;

    return str;
}

void CPPCodeGenerator::generate_foreach_loop(StatementFunctionNode* node)
{
    CPPExpressionGenerator visitor;
    visitor.variables = this->global_context;
    node->array->accept(visitor);

    int size = 0;

    ArraySizeVisitor size_visitor{};
    size_visitor.variables = global_context;
    node->array->accept(size_visitor);

    TypeLocatingVisitor type_visitor{global_context, nullptr};
    if(size_visitor.first_element == nullptr) {
        return;
    }
    size_visitor.first_element->accept(type_visitor);

    VariableContext* inner_context = new VariableContext();
    inner_context->insert(global_context->begin(), global_context->end());
    
    if(type_visitor.ret_value == VariableType::NUMBER) {
        inner_context->insert({node->internal_variable->s, new VariableInformation(VariableType::NUMBER, new AssignmentNode(node->internal_variable->s, size_visitor.first_element))});
    }else {
        auto var_info = new VariableInformation(VariableType::ARRAY, new AssignmentNode(node->internal_variable->s, size_visitor.first_element));
        var_info->most_recent_assignment_expr = size_visitor.first_element;
        inner_context->insert({node->internal_variable->s, var_info});
    }

    VariableContext *old_context = global_context;

    global_context = inner_context;

    size_visitor.variables = global_context;

    if(node->index_variable != nullptr) {
        str += visitor.pre_statement_setup;
        str += "for(int " + node->index_variable->s + " = 0; " + node->index_variable->s + " < " + std::to_string(size_visitor.size) + "; " + node->index_variable->s + "++) {";
        str += "\n#define " + node->internal_variable->s + " " + visitor.expr + "[" + node->index_variable->s + "]" + "\n";
        
        node->action->accept(*this);
        
        CodeBlockNode* action_block = dynamic_cast<CodeBlockNode*>(node->action);
        if(action_block == nullptr) {
            str += ";";
        }
        str += "\n#undef " + node->internal_variable->s + "\n";
    }else {
        str += visitor.pre_statement_setup;
        std::string idx_var = generate_safe_variable_name();
        str += "for(int " + idx_var + " = 0; " + idx_var + " < " + std::to_string(size_visitor.size) + "; " + idx_var + "++) {";
        str += "\n#define " + node->internal_variable->s + " " + visitor.expr + "[" + idx_var + "]" + "\n";
        node->action->accept(*this);
        CodeBlockNode* action_block = dynamic_cast<CodeBlockNode*>(node->action);
        if(action_block == nullptr) {
            str += ";";
        }
        str += "\n#undef " + node->internal_variable->s + "\n";
    }
    
    
    global_context = old_context;
    delete inner_context;
    

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
        visitor.variables = this->global_context;
        visitor.codegen = this;
        expr->accept(visitor);
        code.insert(0, visitor.pre_statement_setup);
        code += visitor.expr + "<<";
    }

    code += "std::endl";

    return code;
}

std::string CPPCodeGenerator::generate_element_assignment(ElementAssignmentNode *node)
{
    auto indexVisitor = CPPExpressionGenerator();
    indexVisitor.variables = this->global_context;
    indexVisitor.codegen = this;
    node->index->accept(indexVisitor);

    auto valueVisitor = CPPExpressionGenerator();
    valueVisitor.variables = this->global_context;
    indexVisitor.codegen = this;
    node->value->accept(valueVisitor);

    // intentionally ignore pre statement setup here for index because it should never be an array.

    std::string str = valueVisitor.pre_statement_setup + node->name + "[" + indexVisitor.expr + "] " + "=" + valueVisitor.expr;

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
    ArraySizeVisitor size_visitor{};

    size_visitor.variables = variables;

    node->accept(size_visitor);
    
    std::string stars = "";

    for(int i = 0; i < size_visitor.depth - 1; i++) {
        stars += "*";
    }

    expr += "new double" + stars + "[" + std::to_string(size_visitor.size) + "] {";

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
    TypeLocatingVisitor vis = TypeLocatingVisitor(variables, nullptr);
    node->array->accept(vis);
    ArraySizeVisitor arr_vis{};
    arr_vis.variables = variables;
    node->array->accept(arr_vis);

    if(node->type == ExpressionFunctionType::MAP) {
        std::string temp = this->codegen->generate_safe_variable_name();
        ArrayElements* elem = new ArrayElements();

        for(int i = 0; i < arr_vis.size; i++) {
            if(i == 0 && arr_vis.depth > 1) {
                ArrayElements* outer = elem;
                for(int depth = 0; depth < arr_vis.depth - 1; depth++) {
                    outer = new ArrayElements();
                    elem->expressions.push_back(new ArrayNode(outer));
                }
                continue;
            }
            elem->expressions.push_back(new LiteralNumberNode(0));
        }

        ArrayNode* arr_expr = new ArrayNode(elem);

        VariableInformation* info = new VariableInformation();
        info->first_assignment = new AssignmentNode(temp, arr_expr);
        info->array_depth = arr_vis.depth;
        info->type = VariableType::ARRAY;
        info->most_recent_assignment_expr = arr_expr;

        pre_statement_setup += codegen->generate_variable_declaration(temp, info) + ";\n";
        
        delete elem;
        delete info->first_assignment;
        delete arr_expr;
        delete info;

        std::string index_var = this->codegen->generate_safe_variable_name();
        if(node->index_variable != nullptr) {
            index_var = node->index_variable->s;
        }

        CPPExpressionGenerator arr_gen = CPPExpressionGenerator();
        arr_gen.variables = variables;
        node->array->accept(arr_gen);

        VariableNode* var = dynamic_cast<VariableNode*>(node->array);
        std::string temp_mapped_array;
        if(var == nullptr) {
            temp_mapped_array = codegen->generate_safe_variable_name();
            VariableInformation* mapped_array_info = new VariableInformation();
            mapped_array_info->first_assignment = new AssignmentNode(temp_mapped_array, node->array);
            mapped_array_info->array_depth = arr_vis.depth;
            mapped_array_info->type = VariableType::ARRAY;
            mapped_array_info->most_recent_assignment_expr = node->array;

            pre_statement_setup += codegen->generate_variable_declaration(temp, mapped_array_info);
            delete mapped_array_info;
        }

        pre_statement_setup += "for(int " + index_var + " = 0; " + index_var + " < " + std::to_string(arr_vis.size) + "; " + index_var + "++) {\n";
        pre_statement_setup += arr_gen.pre_statement_setup;
        if(node->internal_variable != nullptr) {
            if(var != nullptr) {
                pre_statement_setup += "\n#define " + node->internal_variable->s + " " + var->s + "[" + index_var + "]\n";
            }else {
                pre_statement_setup += "\n#define " + node->internal_variable->s + " " + temp_mapped_array + "[" + index_var + "]\n";
            }
        }

        CPPExpressionGenerator action_gen = CPPExpressionGenerator();
        action_gen.variables = variables;
        node->action->accept(action_gen);


        pre_statement_setup += action_gen.pre_statement_setup + "\n" + temp + "[" + index_var + "] = " + action_gen.expr + ";";
        
        if(node->internal_variable != nullptr) {
            pre_statement_setup += "\n#undef " + node->internal_variable->s + "\n";
        }

        pre_statement_setup += "}";

        expr += temp;
    }
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
