#include "tree.hpp"
#include "generated/parser.tab.hpp"
#include <iostream>
#include "preprocessor.hpp"
#include "codegen.hpp"
#include <fstream>
#include <cstdlib>
#include <fstream>
#include <filesystem>

void scan_string(const char* str);
extern int yydebug;
StatementNode* getRoot();

#define PREPROCESS
#define CODEGEN

int main(int argc, char* argv[]) {
    if(argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.rcy>" << std::endl;
        return 1;
    }

    std::fstream codeFile{};
    codeFile.open(argv[1]);
    yydebug = 0;

    std::string code_content;

    if(codeFile.is_open()) {
        std::string line;
        while(std::getline(codeFile, line)) {
            code_content += line + "\n";
        }
    } else {
        std::cerr << "Failed to open the file: " << argv[1] << std::endl;
        return 1;
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
        return 1;
    }

    std::cout << *root << std::endl;

    #ifdef PREPROCESS
    if(res.messages.size() > 0) {
        for(auto& message : res.messages) {
            std::cout << message << std::endl;
        }
        return 1;
    } else {
        std::cout << "No errors found." << std::endl;
    }

    #ifdef CODEGEN

    CPPCodeGenerator codegen{};
    std::string code = codegen.generate(root, res);

    std::cout << code << std::endl;

    // Write to out.cpp
    std::ofstream file;
    file.open("out.cpp");
    file << code;
    file.close();

    // Extract filename without extension
    std::string input_file(argv[1]);
    std::string filename_without_extension = input_file.substr(0, input_file.find_last_of('.'));

    // Create the output executable name with "_rcy.exe"
    std::string output_executable = filename_without_extension + "_rcy.exe";

    // Compile the generated code into the desired output executable
    std::string compile_command = "g++ -O2 out.cpp -o " + output_executable;
    system(compile_command.c_str());

    // Delete the temporary out.cpp file
    // std::filesystem::remove("out.cpp");

    std::cout << "Executable generated: " << output_executable << std::endl;

    #endif
    #endif

    return 0;
}