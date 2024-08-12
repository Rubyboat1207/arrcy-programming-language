#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>

void scan_string(const char* str);
StatementNode* getRoot();

int main() {
    scan_string("x = 2 / 2; y = 2;");
    yyparse();
    std::cout << *getRoot() << std::endl;
    return 0;
}
