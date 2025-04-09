#include "lexer.h"
#include <cctype>

const std::unordered_set<std::string> keywords = {
    "int", "bool", "if", "then", "else", "while", "do", "read", "write"
};

const std::unordered_set<std::string> operators = {
    "+", "-", "*", "/", ">", ">=", "<", "<=", "==", "!=", "||", "&&", "!", "=", ":="
};

Lexer::Lexer(const std::string& source) : source(source), position(0) {}

bool Lexer::isKeyword(const std::string& word) {
    return keywords.find(word) != keywords.end();
}

bool Lexer::isOperator(const std::string& op) {
    return operators.find(op) != operators.end();
}

bool Lexer::isNumber(char ch) {
    return isdigit(ch);
}

bool Lexer::isIdentifierStart(char ch) {
    return isalpha(ch) || ch == '_';
}

bool Lexer::isIdentifierChar(char ch) {
    return isalnum(ch) || ch == '_';
}

bool Lexer::isBoolean(const std::string& word) {
    return word == "true" || word == "false";
}

void Lexer::skipWhitespace() {
    while (position < source.length()) {
        if (isspace(source[position])) {
            position++;
        } else if (source.substr(position, 2) == "//") {
            position = source.find('\n', position);
            if (position == std::string::npos) {
                position = source.length();
            } else {
                position++;
            }
        } else if (source.substr(position, 2) == "/*") {
            position = source.find("*/", position);
            if (position == std::string::npos) {
                position = source.length();
            } else {
                position += 2;
            }
        } else {
            break;
        }
    }
}

Token Lexer::parseKeywordOrIdentifier() {
    size_t start = position;
    while (position < source.length() && isIdentifierChar(source[position])) {
        position++;
    }
    std::string word = source.substr(start, position - start);

    if (isKeyword(word)) {
        return {TokenType::KEYWORD, word};
    } else if (isBoolean(word)) {
        return {TokenType::BOOLEAN, word};
    }
    return {TokenType::IDENTIFIER, word};
}

Token Lexer::parseNumber() {
    size_t start = position;
    while (position < source.length() && isNumber(source[position])) {
        position++;
    }
    std::string num = source.substr(start, position - start);
    return {TokenType::NUMBER, num};
}

Token Lexer::parseBoolean() {
    size_t start = position;
    while (position < source.length() && isalpha(source[position])) {
        position++;
    }
    std::string boolStr = source.substr(start, position - start);
    if (isBoolean(boolStr)) {
        return {TokenType::BOOLEAN, boolStr};
    }
    return {TokenType::ERROR, boolStr};
}

Token Lexer::parseOperator() {
    if (position + 1 < source.length()) {
        std::string twoCharOp = source.substr(position, 2);
        if (isOperator(twoCharOp)) {
            position += 2;
            return {TokenType::OPERATOR, twoCharOp};
        }
    }
    
    std::string oneCharOp = std::string(1, source[position]);
    if (isOperator(oneCharOp)) {
        position++;
        return {TokenType::OPERATOR, oneCharOp};
    }
    
    return {TokenType::ERROR, oneCharOp};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (position < source.length()) {
        skipWhitespace();
        if (position >= source.length()) break;

        char current = source[position];
        
        if (isIdentifierStart(current)) {
            tokens.push_back(parseKeywordOrIdentifier());
        } else if (isNumber(current) || (current == '-' && position + 1 < source.length() && isNumber(source[position + 1]))) {
            tokens.push_back(parseNumber());
        } else if (isOperator(std::string(1, current)) || 
                  (position + 1 < source.length() && isOperator(source.substr(position, 2)))) {
            tokens.push_back(parseOperator());
        } else if (current == '{') {
            tokens.push_back({TokenType::LEFT_BRACE, "{"});
            position++;
        } else if (current == '}') {
            tokens.push_back({TokenType::RIGHT_BRACE, "}"});
            position++;
        } else if (current == ';') {
            tokens.push_back({TokenType::SEMICOLON, ";"});
            position++;
        } else if (current == ',') {
            tokens.push_back({TokenType::COMMA, ","});
            position++;
        } else if (current == '(') {
            tokens.push_back({TokenType::LEFT_PAREN, "("});
            position++;
        } else if (current == ')') {
            tokens.push_back({TokenType::RIGHT_PAREN, ")"});
            position++;
        } else {
            tokens.push_back({TokenType::ERROR, std::string(1, current)});
            position++;
        }
    }
    return tokens;
}