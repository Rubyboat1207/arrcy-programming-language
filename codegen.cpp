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

    VariableContext* inner_context = new VariableContext();
    inner_context->insert(global_context->begin(), global_context->end());
    VariableInformation* var_info;
    if(size_visitor.depth == 1) {
        var_info = new VariableInformation(VariableType::NUMBER, new AssignmentNode(node->internal_variable->s, size_visitor.first_element));
    }else {
        var_info = new VariableInformation(VariableType::ARRAY, nullptr);
        var_info->array_depth = size_visitor.depth - 1;
        for(int i = 1; i < size_visitor.depth; i++) {
            if(i == 8) {
                break;
            }
            var_info->opt_array_sizes[i - 1] = size_visitor.sizes[i];
        }
        var_info->uses_array_size = true;
    }

    var_info->most_recent_assignment_expr = nullptr;
    inner_context->insert({node->internal_variable->s, var_info});
    auto pp_result = preprocessor::preprocess_with_ctx(node->action, inner_context);

    std::vector<VariableInformation*> added_info{};

    if(pp_result.opt_variables.has_value()) {
        for(const auto v : *pp_result.opt_variables.value()) {
            if(inner_context->find(v.first) == inner_context->end()) {
                added_info.push_back(v.second);
                inner_context->insert(v);
            }
        }
    }

    VariableContext *old_context = global_context;

    global_context = inner_context;

    size_visitor.variables = global_context;


    if(node->index_variable != nullptr) {
        str += visitor.pre_statement_setup;
        str += "for(int " + node->index_variable->s + " = 0; " + node->index_variable->s + " < " + std::to_string(size_visitor.sizes[0]) + "; " + node->index_variable->s + "++) {";
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
        str += "for(int " + idx_var + " = 0; " + idx_var + " < " + std::to_string(size_visitor.sizes[0]) + "; " + idx_var + "++) {";
        str += "\n#define " + node->internal_variable->s + " " + visitor.expr + "[" + idx_var + "]" + "\n";
        node->action->accept(*this);
        CodeBlockNode* action_block = dynamic_cast<CodeBlockNode*>(node->action);
        if(action_block == nullptr) {
            str += ";";
        }
        str += "\n#undef " + node->internal_variable->s + "\n";
    }
    
    
    global_context = old_context;
    for(const auto added : added_info) {
        delete added;
    }
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

    auto assigneeVisitor = CPPExpressionGenerator();
    assigneeVisitor.variables = global_context;
    assigneeVisitor.codegen = this;
    node->assignee->accept(assigneeVisitor);

    // intentionally ignore pre statement setup here for index because it should never be an array.

    std::string str = valueVisitor.pre_statement_setup + assigneeVisitor.pre_statement_setup + assigneeVisitor.expr + "[" + indexVisitor.expr + "] " + "=" + valueVisitor.expr;

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

    expr += "new double" + stars + "[" + std::to_string(size_visitor.sizes[0]) + "] {";

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
        case ExpressionOperation::MOD:       expr += "%"; break;
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

ArrayNode* createArrayWithDepth(int currentDepth, const ArraySizeVisitor& arr_vis) {
    // Base case: if we are at the maximum depth, return an array filled with zeros
    if (currentDepth == arr_vis.depth) {
        ArrayElements* leafElements = new ArrayElements();

        // Create the elements at the deepest level filled with zeros
        for (int i = 0; i < arr_vis.sizes[currentDepth - 1]; ++i) {
            leafElements->expressions.push_back(new LiteralNumberNode(0));
        }

        return new ArrayNode(leafElements);
    }

    // Otherwise, create an array where each element is another sub-array
    ArrayElements* currentLevelElements = new ArrayElements();

    for (int i = 0; i < arr_vis.sizes[currentDepth - 1]; ++i) {
        // Recursively create sub-arrays
        currentLevelElements->expressions.push_back(createArrayWithDepth(currentDepth + 1, arr_vis));
    }

    return new ArrayNode(currentLevelElements);
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
        ArrayNode* arr_expr = createArrayWithDepth(1, arr_vis);

        VariableInformation* info = new VariableInformation();
        info->first_assignment = new AssignmentNode(temp, arr_expr);
        info->array_depth = arr_vis.depth;
        info->type = VariableType::ARRAY;
        info->most_recent_assignment_expr = arr_expr;

        pre_statement_setup += codegen->generate_variable_declaration(temp, info) + ";\n";
        
        delete info->first_assignment;
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

        pre_statement_setup += "for(int " + index_var + " = 0; " + index_var + " < " + std::to_string(arr_vis.sizes[0]) + "; " + index_var + "++) {\n";
        pre_statement_setup += arr_gen.pre_statement_setup;
        if(var != nullptr) {
            pre_statement_setup += "\n#define " + node->internal_variable->s + " " + var->s + "[" + index_var + "]\n";
        }else {
            pre_statement_setup += "\n#define " + node->internal_variable->s + " " + temp_mapped_array + "[" + index_var + "]\n";
        }

        CPPExpressionGenerator* action_gen = make_child();

        VariableInformation* internal_var_info = new VariableInformation();
        internal_var_info->array_depth = arr_vis.depth - 1;
        internal_var_info->first_assignment = nullptr;
        internal_var_info->most_recent_assignment_expr = arr_expr->values->expressions[0];
        internal_var_info->type = arr_vis.depth - 1 == 0 ? VariableType::NUMBER : VariableType::ARRAY;



        (*action_gen->variables)[node->internal_variable->s] = internal_var_info;
        VariableInformation* index_var_info = nullptr;

        if(node->index_variable != nullptr) {
            index_var_info = new VariableInformation();
            internal_var_info->array_depth = 0;
            internal_var_info->first_assignment = nullptr;
            internal_var_info->most_recent_assignment_expr = nullptr;
            internal_var_info->type = VariableType::NUMBER;
        }
        node->action->accept(*action_gen);

        // so theres a memory leak here, as far as im aware. I dont know how to fix it
        // i also dont have time. so...
        delete arr_expr;
        delete internal_var_info;
        if(index_var_info != nullptr) {
            delete index_var_info;
        }

        pre_statement_setup += action_gen->pre_statement_setup + "\n" + temp + "[" + index_var + "] = " + action_gen->expr + ";";

        delete action_gen->variables;
        delete action_gen;
        
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

CPPExpressionGenerator *CPPExpressionGenerator::make_child()
{
    CPPExpressionGenerator* gen = new CPPExpressionGenerator();

    gen->variables = new VariableContext();
    for(auto v : *variables) {
        (*gen->variables)[v.first] = v.second;
    }
    gen->codegen = codegen;

    return gen;
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
