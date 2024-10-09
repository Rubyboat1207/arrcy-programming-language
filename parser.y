%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <memory>
#include <stdbool.h>
#include "../tree.hpp"

void yyerror(const char *s);
int yylex(void);

StatementNode* root;


%}

%debug

%union {
    double float_val;
    char* str;
    char operation;
    LiteralNumberNode* number;
    StatementNode* stmt;
    ExpressionNode* expr;
    BinOpNode* binop;
    ArrayElements* expressions;
    ArrayNode* arr;
    VariableNode* var;
    ModificationAssign* mod;
    FunctionCallData* fcd;
}

%token <float_val> NUMBER
%token <str> IDENT
%token END_OF_FILE ASSIGN LBRACKET RBRACKET COMMA LPAREN RPAREN LESS_THAN GREATER_THAN LCURLY RCURLY EQUALITY INEQUALITY
%token SLASH PLUS MINUS STAR HASH
%type <number> number_literal
%type <stmt> statement for_each statements
%type <expr> expression array_access expression_function
%type <binop> binary_operation
%type <expressions> elements
%type <arr> array;
%type <var> identifier
%type <mod> modification_assignment
%type <fcd> function_call

%token LEN DIM

%start outer_statements

%%

outer_statements:
    statements END_OF_FILE {
        root = $1;
        YYACCEPT;
    }

statements:
    statement {
        $$ = $1;
    } |
    statements ';' statement {
        $$ = $1->append($3);
    }
    ;

modification_assignment:
    PLUS ASSIGN expression {
        $$ = new ModificationAssign(ExpressionOperation::ADD, $3);
    } |
    MINUS ASSIGN expression {
        $$ = new ModificationAssign(ExpressionOperation::SUBTRACT, $3);
    } |
    SLASH ASSIGN expression {
        $$ = new ModificationAssign(ExpressionOperation::DIVIDE, $3);
    } |
    STAR ASSIGN expression {
        $$ = new ModificationAssign(ExpressionOperation::MULTIPLY, $3);
    }
    ;

statement:
    IDENT ASSIGN expression {
        $$ = new AssignmentNode($1, $3);
    } |
    IDENT LBRACKET expression RBRACKET ASSIGN expression {
        $$ = new ElementAssignmentNode($1, $6, $3);
    } |
    IDENT LBRACKET expression RBRACKET modification_assignment {
        $$ = new ElementAssignmentNode($1, new BinOpNode(new BinOpNode(new VariableNode($1), $3, ExpressionOperation::ACCESS), $5->value, $5->operation), $3);
    } |
    IDENT modification_assignment {
        $$ = new AssignmentNode($1, new BinOpNode(new VariableNode($1), $2->value, $2->operation));
        delete $2;
    } |
    for_each |
    function_call {
        $$ = $1->createNodeStatement();
        
        delete $1;
    } |
    %empty {
        $$ = nullptr;
    }
    ;

identifier:
    IDENT {
        $$ = new VariableNode($1);
    }
    ;

expression:
    array |
    number_literal |
    binary_operation |
    identifier |
    expression_function |
    property_access |
    function_call {
        $$ = $1->createNodeExpression();
        
        delete $1;
    } |
    array_access
    ;

array_access: 
    IDENT LBRACKET expression RBRACKET { 
        $$ = new BinOpNode(new VariableNode($1), $3, ExpressionOperation::ACCESS);
    }
    ;

expression_function:
    LPAREN expression RPAREN HASH identifier LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::MAP, $2, $5, nullptr, $7);
    } | // (x)#a{ a + 5 };
    LPAREN expression RPAREN HASH identifier COMMA identifier LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::MAP, $2, $5, $7, $9);
    } | // (x)#a, i{ a + 5 };
    LPAREN expression RPAREN SLASH identifier LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::FILTER, $2, $5, nullptr, $7);
    } | // (x)/a{ a + 5 };
    LPAREN expression RPAREN SLASH identifier COMMA identifier LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::FILTER, $2, $5, $7, $9);
    }  | // (x)/a{ a + 5 };
    LPAREN expression RPAREN LESS_THAN identifier COMMA identifier LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::REDUCE, $2, $5, $7, $9);
    }  // (x)<acc, v { acc + v };
    ;

for_each:
    LPAREN expression RPAREN STAR identifier COMMA identifier LCURLY statements RCURLY { 
        $$ = new StatementFunctionNode($2, $5, $7, $9);
    } |
    LPAREN expression RPAREN STAR identifier LCURLY statements RCURLY {
        $$ = new StatementFunctionNode($2, $5, nullptr, $7);
    }
    ;

property_access:
    IDENT '.' LEN { printf("length access of %s\n", $1); } | 
    IDENT '.' DIM { printf("dim access of %s\n", $1); }
    ;

function_call:
    IDENT LPAREN elements RPAREN { 
        $$ = new FunctionCallData(std::string($1), $3);
    } |
    IDENT LPAREN RPAREN { 
        $$ = new FunctionCallData(std::string($1), nullptr);
    }
    ;

binary_operation:
    expression SLASH expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::DIVIDE);
    } |
    expression PLUS  expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::ADD);
    } |
    expression MINUS expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::SUBTRACT);
    } |
    expression STAR expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::MULTIPLY);
    } |
    expression LESS_THAN expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::LT);
    } |
    expression GREATER_THAN expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::GT);
    } |
    expression GREATER_THAN ASSIGN expression { 
        $$ = new BinOpNode($1, $4, ExpressionOperation::GTEQ);
    } |
    expression LESS_THAN ASSIGN expression { 
        $$ = new BinOpNode($1, $4, ExpressionOperation::LTEQ);
    } |
    expression EQUALITY expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::EQ);
    } |
    expression INEQUALITY expression { 
        $$ = new BinOpNode($1, $3, ExpressionOperation::INEQ);
    } 
    ;

number_literal:
    NUMBER { 
        $$ = new LiteralNumberNode($1);
    }

array:
    LBRACKET elements RBRACKET {
        $$ = new ArrayNode($2);
    } |
    LBRACKET RBRACKET { 
        $$ = new ArrayNode(nullptr);
    }
    ;

elements:
    expression {
        $$ = new ArrayElements();
        $$->expressions.push_back($1);
    } |
    elements COMMA expression {
        $1->expressions.push_back($3);

        $$ = $1;
    }
    ;

%%
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

StatementNode* getRoot() {
    return root;
}