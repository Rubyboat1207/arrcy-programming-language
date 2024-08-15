#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>
#include "preprocessor.hpp"

void scan_string(const char* str);
StatementNode* getRoot();

#define PREPROCESS

int main() {
    scan_string(
        "x = 2;"
        "y = [3];"
        "x = max(1, x);"
        "print(x);"
    );
    yyparse();
    auto root = getRoot();
    #ifdef PREPROCESS
    auto res = preprocessor::preprocess(root);
    #endif
    std::cout << *getRoot() << std::endl;
    #ifdef PREPROCESS
    if(res.messages.size() > 0) {
        for(auto message : res.messages) {
            std::cout << message << std::endl;
        }
    }else {
        std::cout << "No errors found." << std::endl;
    }
    #endif
    return 0;
}
