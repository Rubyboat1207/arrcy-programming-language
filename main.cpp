#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>
#include "preprocessor.hpp"
#include "codegen.hpp"
#include <fstream>
#include <cstdlib>
#include <fstream>

void scan_string(const char* str);
StatementNode* getRoot();

#define PREPROCESS
#define CODEGEN

int main(int argc, char* argv[]) {
    std::fstream codeFile{};
    codeFile.open(argv[1]);

    std::string code_content;

    

    if(codeFile.is_open()) {
        std::string line;
        while(std::getline(codeFile, line)) {
            code_content += line + "\n";
        }
    }

    codeFile.close();

    std::cout << code_content << std::endl;

    scan_string(code_content.c_str());
    yyparse();
    auto root = getRoot();
    #ifdef PREPROCESS
    auto res = preprocessor::preprocess(root);
    #endif
    if(root == nullptr) {
        std::cout << "root is null. crash will happen soon." << std::endl;
    }
    std::cout << *root << std::endl;
    #ifdef PREPROCESS
    if(res.messages.size() > 0) {
        for(auto message : res.messages) {
            std::cout << message << std::endl;
        }
        return 1;
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

    system("g++ -O2 out.cpp -o out.exe");

    #endif
    #endif
    
    return 0;
}
