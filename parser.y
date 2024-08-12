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
}

%token <float_val> NUMBER
%token <str> IDENT
%token END_OF_FILE ASSIGN LBRACKET RBRACKET COMMA LPAREN RPAREN LESS_THAN GREATER_THAN LCURLY RCURLY EQUALITY INEQUALITY
%token SLASH PLUS MINUS STAR HASH
%type <number> number_literal
%type <stmt> statement
%type <expr> expression
%type <binop> binary_operation

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
        $$ = new AssignmentNode(std::string($1), $3);
    } |
    for_each |
    function_call |
    %empty |
    END_OF_FILE {
        std::cout << "ENDED FILE" << std::endl;
        YYACCEPT;
    }
    ;

expression:
    array |
    number_literal { $$ = $1; } |
    binary_operation |
    IDENT { printf("identifier %s\n", $1); } |
    expression_function |
    property_access |
    function_call |
    array_access
    ;

array_access: 
    IDENT LBRACKET expression RBRACKET { printf("array access of %s\n", $1); }
    ;

expression_function:
    LPAREN IDENT RPAREN HASH IDENT LCURLY expression RCURLY { printf("mapping function with internal var called %s\n", $5); } | // (x)#a{ a + 5 };
    LPAREN IDENT RPAREN HASH IDENT COMMA IDENT LCURLY expression RCURLY { printf("mapping function with internal var called %s, %s\n", $5, $7); } | // (x)#a{ a + 5 };
    LPAREN IDENT RPAREN SLASH IDENT LCURLY expression RCURLY { printf("filtering function with internal var called %s\n", $5); } | // (x)/a{ a + 5 };
    LPAREN IDENT RPAREN SLASH IDENT COMMA IDENT LCURLY expression RCURLY { printf("filtering function with internal var called %s, %s\n", $5, $7); } | // (x)/a{ a + 5 };
    LPAREN IDENT RPAREN LESS_THAN IDENT COMMA IDENT GREATER_THAN LCURLY expression RCURLY { printf("reduce function with internal vars called %s, %s\n", $5, $7); } // (x)<acc, v>{ acc + v };
    ;

for_each:
    LPAREN IDENT RPAREN STAR IDENT COMMA IDENT STAR LCURLY statements RCURLY { printf("for each function with value and index vars called %s, %s\n", $5, $7); } |
    LPAREN IDENT RPAREN STAR IDENT LCURLY statements RCURLY { printf("for each function with value var called %s\n", $5); }
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
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::DIVIDE;
    } |
    expression PLUS  expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::ADD;
    } |
    expression MINUS expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::SUBTRACT;
    } |
    expression STAR expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::MULTIPLY;
    } |
    expression LESS_THAN expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::LT;
    } |
    expression GREATER_THAN expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::GT;
    } |
    expression GREATER_THAN ASSIGN expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $4;
        $$->operation = ExpressionOperation::GTEQ;
    } |
    expression LESS_THAN ASSIGN expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $4;
        $$->operation = ExpressionOperation::LTEQ;
    } |
    expression EQUALITY expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::EQ;
    } |
    expression INEQUALITY expression { 
        $$ = new BinOpNode();
        $$->a = $1;
        $$->b = $3;
        $$->operation = ExpressionOperation::INEQ;
    } 
    ;

number_literal:
    NUMBER { 
        $$ = new LiteralNumberNode($1);
    }

array:
    LBRACKET elements RBRACKET { printf("Array with elements\n"); } |
    LBRACKET RBRACKET { printf("empty array\n"); }
    ;

elements:
    expression |
    elements COMMA expression
    ;

%%
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}

StatementNode* getRoot() {
    return root;
}