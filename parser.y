%{
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <stdbool.h>
#include "../tree.hpp"

void yyerror(const char *s);
int yylex(void);

StatementNode root = StatementNode();

%}

%union {
    double float_val;
    char* str;
    char operation;
}

%token <float_val> NUMBER
%token <str> IDENT
%token ASSIGN LBRACKET RBRACKET COMMA LPAREN RPAREN LESS_THAN GREATER_THAN LCURLY RCURLY EQUALITY INEQUALITY
%token SLASH PLUS MINUS STAR HASH

%type <float_val> number_literal

%token LEN DIM

%start statements

%%
statements:
    statement |
    statements ';' statement
    ;

assignment:
    ASSIGN |
    PLUS ASSIGN |
    MINUS ASSIGN |
    SLASH ASSIGN |
    STAR ASSIGN
    ;

statement:
    IDENT assignment expression |
    for_each |
    function_call |
    %empty
    ;

expression:
    array |
    number_literal |
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
    expression SLASH expression { printf("binop divide\n"); } |
    expression PLUS  expression { printf("binop add\n"); } |
    expression MINUS expression { printf("binop subtract\n"); } |
    expression STAR  expression { printf("binop multiply\n"); } |
    expression LESS_THAN  expression { printf("binop lt\n"); } |
    expression GREATER_THAN  expression { printf("binop gt\n"); } |
    expression GREATER_THAN ASSIGN  expression { printf("binop gt or eq\n"); } |
    expression LESS_THAN ASSIGN  expression { printf("binop lt or eq\n"); } |
    expression EQUALITY expression { printf("binop equality\n"); } |
    expression INEQUALITY expression { printf("binop inequality\n"); } 
    ;

number_literal:
    NUMBER { $$ = $1; }

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
