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
}

%token <float_val> NUMBER
%token <str> IDENT
%token END_OF_FILE ASSIGN LBRACKET RBRACKET COMMA LPAREN RPAREN LESS_THAN GREATER_THAN LCURLY RCURLY EQUALITY INEQUALITY
%token SLASH PLUS MINUS STAR HASH
%type <number> number_literal
%type <stmt> statement
%type <expr> expression array_access expression_function
%type <binop> binary_operation
%type <expressions> elements
%type <arr> array;
%type <var> identifier

%token LEN DIM

%start statements

%%

statements:
    statement {
        root = $1;
    } |
    statements ';' statement {
        if(root == NULL) {
            root = $3;
        }else {
            root = root->append($3);
        }
    }
    ;

assignment:
    ASSIGN |
    PLUS ASSIGN |
    MINUS ASSIGN |
    SLASH ASSIGN |
    STAR ASSIGN
    ;

statement:
    IDENT assignment expression {
        $$ = new AssignmentNode($1, $3);
    } |
    for_each |
    function_call |
    %empty |
    END_OF_FILE {
        std::cout << "ENDED FILE" << std::endl;
        YYACCEPT;
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
    function_call |
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
    LPAREN expression RPAREN LESS_THAN identifier COMMA identifier GREATER_THAN LCURLY expression RCURLY { 
        $$ = new ExpressionFunctionNode(ExpressionFunctionType::REDUCE, $2, $5, $7, $10);
    }  // (x)<acc, v>{ acc + v };
    ;

for_each:
    LPAREN expression RPAREN STAR IDENT COMMA IDENT STAR LCURLY statements RCURLY { printf("for each function with value and index vars called %s, %s\n", $5, $7); } |
    LPAREN expression RPAREN STAR IDENT LCURLY statements RCURLY { printf("for each function with value var called %s\n", $5); }
    ;

property_access:
    IDENT '.' LEN { printf("length access of %s\n", $1); } | 
    IDENT '.' DIM { printf("dim access of %s\n", $1); }
    ;

function_call:
    IDENT LPAREN expression RPAREN { printf("function call %s\n", $1); } |
    IDENT LPAREN RPAREN { printf("function call %s\n", $1); }
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