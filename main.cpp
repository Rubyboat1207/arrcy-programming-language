#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>

void scan_string(const char* str);

int main() {
    scan_string("x = 2;");
    yyparse();
    return 0;
}
