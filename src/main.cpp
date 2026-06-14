#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib> // Required for std::system()
#include <cstdio>  // Required for std::remove()

// Include our transpiler engine phases
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/generator.h"

// Helper function to read a file from the hard drive into a std::string
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Helper function to write the generated C++ string to a file
void writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to file '" << filepath << "'\n";
        exit(1);
    }
    file << content;
}

int main(int argc, char* argv[]) {
    std::string sourceCode;
    
    if (argc != 2) {
        std::cerr << "Usage: volt <script.js>\n";
        return 1;
    }

    try {
        sourceCode = readFile(argv[1]);
    } catch (...) {
        std::cerr << "Error: Could not read file '" << argv[1] << "'.\n";
        return 1;
    }

    std::string tempCppFile = ".temp_output.cpp";
    std::string tempExecFile = ".temp_exec.exe"; 

    try {
        Lexer lexer(sourceCode);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(std::move(tokens));
        auto ast = parser.parse();

        Generator generator;
        std::string cppCode = generator.generate(ast);

        writeFile(tempCppFile, cppCode);

        // Compile against the static library and execute without wrapper text
        std::string compileAndRunCmd = "g++ -std=c++17 -O3 " + tempCppFile + 
            " -I src/lib -L. -lvoltjs -o " + tempExecFile + " && " + tempExecFile;

        int result = std::system(compileAndRunCmd.c_str());

        std::remove(tempCppFile.c_str());
        std::remove(tempExecFile.c_str());

        if (result != 0) {
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Syntax/Runtime Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}