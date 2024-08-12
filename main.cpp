#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>

void scan_string(const char* str);
StatementNode* getRoot();

int main() {
    scan_string(
        "x = 2 / 2 + 5;"
        "array = [0, 1, 2];"
        "test = 5 + y;"
        "access = array[2];"
        "arr2 = (array)#x{ x + 1 };"
        "literal_test = ([2, 3, 12, 5])#x{ x + 1 };"
        "(array)*x{y = y + x;};"
        "y -= 2;"
        "y += 2;"
        "y /= 2;"
        "y *= 2;"
    );
    yyparse();
    auto root = getRoot();
    std::cout << *getRoot() << std::endl;
    return 0;
}
