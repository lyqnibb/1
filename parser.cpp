#include "parser.h"
#include <iostream>
#include <memory>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), position(0) {}

Token Parser::currentToken() {
    if (position < tokens.size()) {
        return tokens[position];
    }
    return {TokenType::ERROR, ""};
}

void Parser::consume() {
    if (position < tokens.size()) {
        position++;
    }
}

bool Parser::match(TokenType type) {
    if (currentToken().type == type) {
        consume();
        return true;
    }
    return false;
}

std::unique_ptr<ASTNode> Parser::parse() {
    return program();
}

std::unique_ptr<ASTNode> Parser::program() {
    return statementList();
}

std::unique_ptr<ASTNode> Parser::statementList() {
    auto firstStmt = statement();
    if (!firstStmt) return nullptr;
    
    auto current = std::move(firstStmt);
    while (true) {
        if (match(TokenType::SEMICOLON)) {
            auto nextStmt = statement();
            if (!nextStmt) break;
            
            auto listNode = std::make_unique<ASTNode>(TokenType::SEMICOLON, ";");
            listNode->left = std::move(current);
            listNode->right = std::move(nextStmt);
            current = std::move(listNode);
        } else if (currentToken().type == TokenType::RIGHT_BRACE || 
                  currentToken().type == TokenType::ERROR) {
            break;
        } else {
            auto nextStmt = statement();
            if (!nextStmt) break;
            
            auto listNode = std::make_unique<ASTNode>(TokenType::SEMICOLON, ";");
            listNode->left = std::move(current);
            listNode->right = std::move(nextStmt);
            current = std::move(listNode);
        }
    }
    return current;
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (match(TokenType::LEFT_BRACE)) {
        auto blockNode = std::make_unique<ASTNode>(TokenType::LEFT_BRACE, "{");
        blockNode->left = statementList();
        if (!match(TokenType::RIGHT_BRACE)) {
            return nullptr;
        }
        return blockNode;
    }
    
    if (auto node = declaration()) return node;
    if (auto node = assignment()) return node;
    if (auto node = readStatement()) return node;
    if (auto node = writeStatement()) return node;
    if (auto node = ifStatement()) return node;
    if (auto node = whileStatement()) return node;
    
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::declaration() {
    Token typeToken = currentToken();
    if (typeToken.type == TokenType::KEYWORD && 
        (typeToken.value == "int" || typeToken.value == "bool")) {
        consume();
        
        auto declNode = std::make_unique<ASTNode>(typeToken.type, typeToken.value);
        auto current = declNode.get();
        
        Token idToken = currentToken();
        if (idToken.type != TokenType::IDENTIFIER) {
            return nullptr;
        }
        
        current->left = std::make_unique<ASTNode>(idToken.type, idToken.value);
        consume();
        
        while (match(TokenType::COMMA)) {
            idToken = currentToken();
            if (idToken.type != TokenType::IDENTIFIER) {
                return nullptr;
            }
            
            current->right = std::make_unique<ASTNode>(idToken.type, idToken.value);
            current = current->right.get();
            consume();
        }
        
        return declNode;
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::assignment() {
    Token idToken = currentToken();
    if (idToken.type == TokenType::IDENTIFIER) {
        consume();
        
        Token opToken = currentToken();
        if (opToken.type == TokenType::OPERATOR && opToken.value == "=") {
            consume();
            
            auto expr = expression();
            if (!expr) return nullptr;
            
            auto assignNode = std::make_unique<ASTNode>(opToken.type, opToken.value);
            assignNode->left = std::make_unique<ASTNode>(idToken.type, idToken.value);
            assignNode->right = std::move(expr);
            return assignNode;
        }
        position--;
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::readStatement() {
    if (match(TokenType::KEYWORD) && currentToken().value == "read") {
        consume();
        
        if (match(TokenType::IDENTIFIER)) {
            auto readNode = std::make_unique<ASTNode>(TokenType::KEYWORD, "read");
            readNode->left = std::make_unique<ASTNode>(TokenType::IDENTIFIER, currentToken().value);
            return readNode;
        }
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::writeStatement() {
    if (match(TokenType::KEYWORD) && currentToken().value == "write") {
        consume();
        
        auto expr = expression();
        if (!expr) {
            if (currentToken().type == TokenType::IDENTIFIER || 
                currentToken().type == TokenType::NUMBER) {
                expr = std::make_unique<ASTNode>(currentToken().type, currentToken().value);
                consume();
            }
        }
        
        if (expr) {
            auto writeNode = std::make_unique<ASTNode>(TokenType::KEYWORD, "write");
            writeNode->left = std::move(expr);
            return writeNode;
        }
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::expression() {
    auto left = term();
    if (!left) return nullptr;
    
    while (position < tokens.size()) {
        Token op = currentToken();
        if (op.type == TokenType::OPERATOR && (op.value == "+" || op.value == "-")) {
            consume();
            auto right = term();
            if (!right) return nullptr;
            
            auto newNode = std::make_unique<ASTNode>(op.type, op.value);
            newNode->left = std::move(left);
            newNode->right = std::move(right);
            left = std::move(newNode);
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::term() {
    auto left = factor();
    if (!left) return nullptr;
    
    while (position < tokens.size()) {
        Token op = currentToken();
        if (op.type == TokenType::OPERATOR && (op.value == "*" || op.value == "/")) {
            consume();
            auto right = factor();
            if (!right) return nullptr;
            
            auto newNode = std::make_unique<ASTNode>(op.type, op.value);
            newNode->left = std::move(left);
            newNode->right = std::move(right);
            left = std::move(newNode);
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::factor() {
    Token token = currentToken();
    if (token.type == TokenType::NUMBER || token.type == TokenType::IDENTIFIER) {
        consume();
        return std::make_unique<ASTNode>(token.type, token.value);
    } else if (token.type == TokenType::LEFT_PAREN) {
        consume();
        auto expr = expression();
        if (!expr) return nullptr;
        
        if (!match(TokenType::RIGHT_PAREN)) {
            return nullptr;
        }
        return expr;
    }
    return nullptr;
}

void Parser::printAST(const ASTNode* node, int indent, std::ostream& out) {
    if (!node) return;
    
    for (int i = 0; i < indent; ++i) out << "  ";
    out << "(" << static_cast<int>(node->type) << ", " << node->value << ")\n";
    
    printAST(node->left.get(), indent + 1, out);
    printAST(node->right.get(), indent + 1, out);
}

void Parser::writeASTToFile(const ASTNode* node, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    printAST(node, 0, out);
}