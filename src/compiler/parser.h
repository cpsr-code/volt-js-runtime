#pragma once
#include "lexer.h"
#include "ast_nodes.h"
#include <vector>
#include <memory>
#include <stdexcept>

class Parser {
private:
    std::vector<Token> tokens;
    int current = 0; // The index of the token we are currently looking at

    // ==========================================
    // 1. CORE HELPER METHODS
    // ==========================================
    
    // Looks at the current token without consuming it
    Token peek() const;
    
    // Looks at the previously consumed token
    Token previous() const;
    
    // Checks if we've run out of tokens
    bool isAtEnd() const;
    
    // Checks if the current token matches a specific type
    bool check(TokenType type) const;
    
    // Consumes and returns the current token, advancing the pointer
    Token advance();
    
    // If the current token matches any of the given types, it consumes it and returns true
    bool match(std::initializer_list<TokenType> types);
    
    // Requires the current token to be a specific type, otherwise throws a Syntax Error
    Token consume(TokenType type, const std::string& message);

    // Error handling synchronization (prevents cascading errors if syntax is wrong)
    class ParseError : public std::runtime_error {
        using std::runtime_error::runtime_error;
    };
    ParseError error(Token token, const std::string& message);
    void synchronize();
    bool isArrowFunction() const; // Added for lookahead

    // ==========================================
    // 2. STATEMENT PARSERS (The structure of the code)
    // ==========================================
    
    std::unique_ptr<StmtNode> declaration();
    std::unique_ptr<StmtNode> varDeclaration();
    std::unique_ptr<StmtNode> functionDeclaration();
    
    std::unique_ptr<StmtNode> statement();
    std::unique_ptr<StmtNode> ifStatement();
    std::unique_ptr<StmtNode> forStatement();
    std::unique_ptr<StmtNode> whileStatement();
    std::unique_ptr<StmtNode> doWhileStatement();
    std::unique_ptr<StmtNode> switchStatement();
    std::unique_ptr<StmtNode> returnStatement();
    std::unique_ptr<StmtNode> breakStatement();
    std::unique_ptr<StmtNode> continueStatement();
    std::unique_ptr<BlockStmt> block();
    std::unique_ptr<StmtNode> expressionStatement();

    // ==========================================
    // 3. EXPRESSION PARSERS (Order of Operations)
    // ==========================================
    // These are ordered from lowest precedence to highest precedence.
    
    std::unique_ptr<ExprNode> expression();
    std::unique_ptr<ExprNode> assignment();
    std::unique_ptr<ExprNode> logicalOr();
    std::unique_ptr<ExprNode> logicalAnd();
    std::unique_ptr<ExprNode> bitwiseOr();
    std::unique_ptr<ExprNode> bitwiseXor();
    std::unique_ptr<ExprNode> bitwiseAnd();
    std::unique_ptr<ExprNode> equality();   // ==, ===, !=, !==
    std::unique_ptr<ExprNode> comparison(); // <, >, <=, >=
    std::unique_ptr<ExprNode> shift();      // <<, >>, >>>
    std::unique_ptr<ExprNode> term();       // +, -
    std::unique_ptr<ExprNode> factor();     // *, /, %
    std::unique_ptr<ExprNode> update();     // ++, --
    std::unique_ptr<ExprNode> unary();      // !, -
    std::unique_ptr<ExprNode> call();       // func(), arr[0], obj.prop
    std::unique_ptr<ExprNode> primary();    // Numbers, Strings, Identifiers, Arrays, Objects

    // Helpers for complex literals
    std::unique_ptr<ExprNode> arrayLiteral();
    std::unique_ptr<ExprNode> objectLiteral();

public:
    Parser(std::vector<Token>&& tokens);

    // The main entry point: Parses all tokens and returns a list of top-level statements
    std::vector<std::unique_ptr<StmtNode>> parse();
};