#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>
#include "preprocessor.hpp"
#include "codegen.hpp"
#include <fstream>
#include <cstdlib>

void scan_string(const char* str);
StatementNode* getRoot();

#define PREPROCESS
#define CODEGEN

int main() {
    scan_string(
        "x = 2;"
        "x += 5;"
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
    #ifdef CODEGEN
    
    CPPCodeGenerator codegen{};

    std::string code = codegen.generate(root, res);

    std::cout << code << std::endl;

    std::ofstream file;
    file.open("out.cpp");
    file << code;
    file.close();

    system("g++ out.cpp -o out.exe");

    #endif
    #endif
    
    return 0;
}
