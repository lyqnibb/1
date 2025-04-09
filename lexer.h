#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <fstream>
#include <map>

enum TokenType {
    KEYWORD, IDENTIFIER, NUMBER, STRING,
    ASSIGN_INT, ASSIGN_BOOL, // = 和 :=
    ADD, SUB, MUL, DIV, MOD, // 算术运算符
    LT, LE, GT, GE, EQ, NE, // 关系运算符
    AND, OR, // 逻辑运算符
    COMMA, SEMICOLON, COLON, // 分隔符
    LPAREN, RPAREN, LBRACE, RBRACE, // 括号
    WHILE, DO, READ, WRITE, // 关键字
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

class Lexer {
public:
    Lexer(const std::string& filename);
    Token nextToken();
    bool hasMoreTokens() { return !eof; }

private:
    std::ifstream file;
    std::string currentLine;
    int lineNumber = 1;
    size_t pos = 0;
    bool eof = false;

    std::map<std::string, TokenType> keywords = {
        {"int", KEYWORD}, {"bool", KEYWORD},
        {"while", WHILE}, {"do", DO},
        {"read", READ}, {"write", WRITE}
    };

    TokenType getTokenType(const std::string& str);
    void skipWhitespace();
    std::string readIdentifier();
    std::string readNumber();
};

#endif