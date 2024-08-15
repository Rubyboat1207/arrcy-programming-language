#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>
#include "preprocessor.hpp"

void scan_string(const char* str);
StatementNode* getRoot();

int main() {
    scan_string(
        "x = 2;"
        "y = [3];"
        "x = min(1, y);"
    );
    yyparse();
    auto root = getRoot();
    auto res = preprocess(root);
    std::cout << *getRoot() << std::endl;

    if(res.messages.size() > 0) {
        for(auto message : res.messages) {
            std::cout << message << std::endl;
        }
    }else {
        std::cout << "No errors found." << std::endl;
    }
    return 0;
}
