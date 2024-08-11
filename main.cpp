#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>

int main() {
    yyparse();  // Parse the input and build the AST
    return 0;
}
