#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <memory>
#include <vector>
#include <iostream>

// AST节点定义
struct ASTNode {
    std::string type;
    std::string value;
    std::vector<std::unique_ptr<ASTNode>> children;

    void print(int indent = 0) const {
        for (int i = 0; i < indent; i++) std::cout << "  ";
        std::cout << type;
        if (!value.empty()) std::cout << "(" << value << ")";
        std::cout << std::endl;
        for (const auto& child : children) {
            child->print(indent + 1);
        }
    }
};

class Parser {
public:
    Parser(Lexer& lexer) : lexer(lexer) { currentToken = lexer.nextToken(); }

    std::unique_ptr<ASTNode> parse() {
        auto program = std::make_unique<ASTNode>();
        program->type = "PROGRAM";
        
        while (currentToken.type != END_OF_FILE) {
            if (currentToken.type == KEYWORD && (currentToken.value == "int" || currentToken.value == "bool")) {
                program->children.push_back(parseDeclaration());
            } else {
                program->children.push_back(parseStatement());
            }
        }
        return program;
    }

private:
    Lexer& lexer;
    Token currentToken;

    void consume(TokenType expected) {
        if (currentToken.type == expected) {
            currentToken = lexer.nextToken();
        } else {
            throw std::runtime_error("语法错误：预期 " + std::to_string(expected) + "，得到 " + std::to_string(currentToken.type));
        }
    }

    // 声明语句：类型 变量列表;
    std::unique_ptr<ASTNode> parseDeclaration() {
        auto decl = std::make_unique<ASTNode>();
        decl->type = "DECLARATION";
        decl->value = currentToken.value; // int 或 bool
        consume(KEYWORD);
        decl->children.push_back(parseVariableList());
        consume(SEMICOLON);
        return decl;
    }

    // 变量列表：标识符 (, 标识符)*
    std::unique_ptr<ASTNode> parseVariableList() {
        auto list = std::make_unique<ASTNode>();
        list->type = "VARIABLE_LIST";
        list->children.push_back(parseIdentifier());
        while (currentToken.type == COMMA) {
            consume(COMMA);
            list->children.push_back(parseIdentifier());
        }
        return list;
    }

    std::unique_ptr<ASTNode> parseIdentifier() {
        auto node = std::make_unique<ASTNode>();
        node->type = "IDENTIFIER";
        node->value = currentToken.value;
        consume(IDENTIFIER);
        return node;
    }

    // 执行语句：赋值、while、read、write、块语句
    std::unique_ptr<ASTNode> parseStatement() {
        if (currentToken.type == IDENTIFIER) {
            return parseAssignment();
        } else if (currentToken.type == WHILE) {
            return parseWhileStatement();
        } else if (currentToken.type == READ || currentToken.type == WRITE) {
            return parseIOStatement();
        } else if (currentToken.type == LBRACE) {
            return parseBlockStatement();
        } else {
            throw std::runtime_error("未知语句类型");
        }
    }

    // 赋值语句：标识符 = 表达式; 或 标识符 := 布尔表达式;
    std::unique_ptr<ASTNode> parseAssignment() {
        auto assign = std::make_unique<ASTNode>();
        assign->type = (currentToken.type == IDENTIFIER) ? "ASSIGNMENT" : "INVALID_ASSIGN";
        auto var = parseIdentifier();
        
        if (currentToken.type == ASSIGN_INT) {
            assign->value = "INT_ASSIGN";
            consume(ASSIGN_INT);
            assign->children.push_back(var);
            assign->children.push_back(parseArithmeticExpression());
        } else if (currentToken.type == ASSIGN_BOOL) {
            assign->value = "BOOL_ASSIGN";
            consume(ASSIGN_BOOL);
            assign->children.push_back(var);
            assign->children.push_back(parseBooleanExpression());
        } else {
            throw std::runtime_error("无效赋值运算符");
        }
        consume(SEMICOLON);
        return assign;
    }

    // while语句：while 条件 do 语句
    std::unique_ptr<ASTNode> parseWhileStatement() {
        auto whileNode = std::make_unique<ASTNode>();
        whileNode->type = "WHILE_STATEMENT";
        consume(WHILE);
        whileNode->children.push_back(parseCondition());
        consume(DO); // 必须匹配do关键字
        whileNode->children.push_back(parseStatement());
        return whileNode;
    }

    // 条件表达式（关系运算）
    std::unique_ptr<ASTNode> parseCondition() {
        auto cond = std::make_unique<ASTNode>();
        cond->type = "CONDITION";
        cond->children.push_back(parseArithmeticExpression());
        cond->children.push_back(parseRelationalOperator());
        cond->children.push_back(parseArithmeticExpression());
        return cond;
    }

    std::unique_ptr<ASTNode> parseRelationalOperator() {
        auto op = std::make_unique<ASTNode>();
        switch (currentToken.type) {
            case LT: op->value = "<"; break;
            case LE: op->value = "<="; break;
            case GT: op->value = ">"; break;
            case GE: op->value = ">="; break;
            case EQ: op->value = "=="; break;
            case NE: op->value = "!="; break;
            default: throw std::runtime_error("无效关系运算符");
        }
        op->type = "RELATIONAL_OP";
        consume(currentToken.type);
        return op;
    }

    // 算术表达式（算符优先简化版，递归下降实现）
    std::unique_ptr<ASTNode> parseArithmeticExpression() {
        return parseTerm();
    }

    std::unique_ptr<ASTNode> parseTerm() {
        auto node = parseFactor();
        while (currentToken.type == ADD || currentToken.type == SUB) {
            auto opNode = std::make_unique<ASTNode>();
            opNode->type = "ARITHMETIC_OP";
            opNode->value = (currentToken.type == ADD) ? "+" : "-";
            opNode->children.push_back(std::move(node));
            consume(currentToken.type);
            opNode->children.push_back(parseFactor());
            node = std::move(opNode);
        }
        return node;
    }

    std::unique_ptr<ASTNode> parseFactor() {
        auto node = std::make_unique<ASTNode>();
        if (currentToken.type == NUMBER) {
            node->type = "NUMBER";
            node->value = currentToken.value;
            consume(NUMBER);
        } else if (currentToken.type == IDENTIFIER) {
            node = parseIdentifier();
        } else if (currentToken.type == LPAREN) {
            consume(LPAREN);
            node = parseArithmeticExpression();
            consume(RPAREN);
        } else {
            throw std::runtime_error("无效表达式因子");
        }
        return node;
    }

    // 布尔表达式（简化版，支持逻辑与或）
    std::unique_ptr<ASTNode> parseBooleanExpression() {
        auto left = parseCondition();
        while (currentToken.type == AND || currentToken.type == OR) {
            auto opNode = std::make_unique<ASTNode>();
            opNode->type = "LOGICAL_OP";
            opNode->value = (currentToken.type == AND) ? "&&" : "||";
            opNode->children.push_back(std::move(left));
            consume(currentToken.type);
            opNode->children.push_back(parseCondition());
            left = std::move(opNode);
        }
        return left;
    }

    // IO语句：read/write 标识符;
    std::unique_ptr<ASTNode> parseIOStatement() {
        auto ioNode = std::make_unique<ASTNode>();
        ioNode->type = (currentToken.type == READ) ? "READ_STATEMENT" : "WRITE_STATEMENT";
        consume(currentToken.type);
        ioNode->children.push_back(parseIdentifier());
        consume(SEMICOLON);
        return ioNode;
    }

    // 块语句：{ 语句* }
    std::unique_ptr<ASTNode> parseBlockStatement() {
        auto block = std::make_unique<ASTNode>();
        block->type = "BLOCK";
        consume(LBRACE);
        while (currentToken.type != RBRACE && currentToken.type != END_OF_FILE) {
            block->children.push_back(parseStatement());
        }
        consume(RBRACE);
        return block;
    }
};

#endif