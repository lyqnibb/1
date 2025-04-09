#include "lexer.h"
#include "parser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        exit(1);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void writeTokensToFile(const std::string& filename, const std::vector<Token>& tokens) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        exit(1);
    }
    for (const auto& token : tokens) {
        file << "(" << static_cast<int>(token.type) << ", " << token.value << ")\n";
    }
}

int main() {
    // 读取输入文件
    std::string source = readFile("source.txt");
    
    // 词法分析
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();
    writeTokensToFile("lex_out.txt", tokens);
    std::cout << "Lexical analysis completed. Results written to lex_out.txt" << std::endl;

    // 语法分析
    Parser parser(tokens);
    std::unique_ptr<ASTNode> ast = parser.parse();
    
    if (ast) {
        // 输出AST到文件
        parser.writeASTToFile(ast.get(), "parse_out.txt");
        std::cout << "Syntax analysis completed. AST written to parse_out.txt" << std::endl;
    } else {
        std::ofstream out("parse_out.txt");
        out << "Syntax analysis failed at position: " << parser.getPosition() << std::endl;
        Token current = parser.currentToken();
        out << "Current token: (" << static_cast<int>(current.type) 
            << ", " << current.value << ")" << std::endl;
        out << "Expected: assignment, declaration, or control statement" << std::endl;
        std::cout << "Syntax analysis failed. See parse_out.txt for details." << std::endl;
    }

    return 0;
}