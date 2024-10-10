%{
#include "../tree.hpp"
#include "parser.tab.hpp"

extern int yyparse(void);

void scan_string(const char* str)
{
    YY_BUFFER_STATE buffer = yy_scan_string(str);
    yy_switch_to_buffer(buffer);
}
%}

%%

[0-9]+"."[0-9]+    { yylval.float_val = atof(yytext); return NUMBER; }
[0-9]+             { yylval.float_val = atof(yytext); return NUMBER; }
"len"              { return LEN; }
"dim"              { return DIM; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.str = strdup(yytext); return IDENT; }
"["                { return LBRACKET; }
"]"                { return RBRACKET; }
","                { return COMMA; }
"=="               { return EQUALITY; }
"!="               { return INEQUALITY; }
"="                { return ASSIGN; }
";"                { return ';'; }
"/"                { return SLASH;}
"+"                { return PLUS;}
"-"                { return MINUS;}
"*"                { return STAR;}
"%"                { return MOD;}
"#"                { return HASH;}
"("                { return LPAREN;}
")"                { return RPAREN;}
"<"                { return LESS_THAN;}
">"                { return GREATER_THAN;}
"{"                { return LCURLY;}
"}"                { return RCURLY;}
<<EOF>>            { return END_OF_FILE;}
"//".*               { /* Do nothing, this is a comment */ }
[ \t\n]+           { /* Skip whitespace */ }
.                  { /* Skip any other character */ }

%%

int yywrap(void) { return 1; }