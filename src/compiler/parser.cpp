#include "parser.h"
#include <iostream>

// ==========================================
// 1. CONSTRUCTOR & CORE HELPERS
// ==========================================

Parser::Parser(std::vector<Token>&& tokens) : tokens(std::move(tokens)) {}

Token Parser::peek() const { return tokens[current]; }
Token Parser::previous() const { return tokens[current - 1]; }
bool Parser::isAtEnd() const { return peek().type == TokenType::EndOfFile; }

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

Parser::ParseError Parser::error(Token token, const std::string& message) {
    std::cerr << "Syntax Error at line " << token.line 
              << ", col " << token.column << ": " << message << std::endl;
    return ParseError(message);
}

// Recovers from an error so the parser doesn't crash on the first typo
void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        switch (peek().type) {
            case TokenType::Let: case TokenType::Const: case TokenType::Function:
            case TokenType::If: case TokenType::For: case TokenType::While:
            case TokenType::Return:
                return;
            default: advance();
        }
    }
}

// Arrow function lookahead: Scans past matching parenthesis to check for '=>'
bool Parser::isArrowFunction() const {
    int temp = current;
    int parens = 1;
    while (temp < tokens.size()) {
        if (tokens[temp].type == TokenType::LParen) parens++;
        else if (tokens[temp].type == TokenType::RParen) {
            parens--;
            if (parens == 0) {
                if (temp + 1 < tokens.size() && tokens[temp + 1].type == TokenType::Arrow) {
                    return true;
                }
                break;
            }
        }
        temp++;
    }
    return false;
}

// ==========================================
// 2. THE MAIN PARSE LOOP
// ==========================================

std::vector<std::unique_ptr<StmtNode>> Parser::parse() {
    std::vector<std::unique_ptr<StmtNode>> statements;
    while (!isAtEnd()) {
        try {
            statements.push_back(declaration());
        } catch (ParseError& error) {
            synchronize(); // Catch errors and attempt to keep parsing
        }
    }
    return statements;
}

// ==========================================
// 3. STATEMENTS (The Structure)
// ==========================================

std::unique_ptr<StmtNode> Parser::declaration() {
    if (match({TokenType::Let, TokenType::Const})) return varDeclaration();
    if (match({TokenType::Function})) return functionDeclaration();
    return statement();
}

std::unique_ptr<StmtNode> Parser::varDeclaration() {
    bool isConst = previous().type == TokenType::Const;
    std::vector<std::pair<Token, std::unique_ptr<ExprNode>>> declarators;
    
    do {
        Token name = consume(TokenType::Identifier, "Expected variable name.");
        std::unique_ptr<ExprNode> initializer = nullptr;
        if (match({TokenType::Assign})) {
            initializer = expression();
        }
        declarators.push_back({name, std::move(initializer)});
    } while (match({TokenType::Comma}));
    
    consume(TokenType::Semicolon, "Expected ';' after variable declaration.");
    return std::make_unique<VariableDeclStmt>(isConst, std::move(declarators));
}

std::unique_ptr<StmtNode> Parser::functionDeclaration() {
    Token name = consume(TokenType::Identifier, "Expected function name.");
    consume(TokenType::LParen, "Expected '(' after function name.");
    
    std::vector<Token> parameters;
    bool hasRest = false;
    Token restParam;
    if (!check(TokenType::RParen)) {
        do {
            if (match({TokenType::Spread})) {
                hasRest = true;
                restParam = consume(TokenType::Identifier, "Expected parameter name after '...'.");
                break; // Rest parameter must be last
            }
            parameters.push_back(consume(TokenType::Identifier, "Expected parameter name."));
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RParen, "Expected ')' after parameters.");
    
    consume(TokenType::LBrace, "Expected '{' before function body.");
    auto body = block();
    
    return std::make_unique<FunctionDeclStmt>(name, std::move(parameters), hasRest, restParam, std::move(body));
}

std::unique_ptr<StmtNode> Parser::statement() {
    if (match({TokenType::If})) return ifStatement();
    if (match({TokenType::For})) return forStatement();
    if (match({TokenType::While})) return whileStatement();
    if (match({TokenType::Do})) return doWhileStatement();
    if (match({TokenType::Switch})) return switchStatement();
    if (match({TokenType::Return})) return returnStatement();
    if (match({TokenType::Break})) return breakStatement();
    if (match({TokenType::Continue})) return continueStatement();
    if (match({TokenType::LBrace})) return block();
    
    return expressionStatement();
}

std::unique_ptr<StmtNode> Parser::ifStatement() {
    consume(TokenType::LParen, "Expected '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RParen, "Expected ')' after if condition.");
    
    auto thenBranch = statement();
    std::unique_ptr<StmtNode> elseBranch = nullptr;
    
    if (match({TokenType::Else})) {
        elseBranch = statement();
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<StmtNode> Parser::whileStatement() {
    consume(TokenType::LParen, "Expected '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RParen, "Expected ')' after while condition.");
    auto body = statement();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<StmtNode> Parser::doWhileStatement() {
    auto body = statement();
    consume(TokenType::While, "Expected 'while' after 'do' block.");
    consume(TokenType::LParen, "Expected '(' after 'while'.");
    auto condition = expression();
    consume(TokenType::RParen, "Expected ')' after do-while condition.");
    consume(TokenType::Semicolon, "Expected ';' after do-while.");
    return std::make_unique<DoWhileStmt>(std::move(body), std::move(condition));
}

std::unique_ptr<StmtNode> Parser::forStatement() {
    consume(TokenType::LParen, "Expected '(' after 'for'.");
    
    std::unique_ptr<StmtNode> initializer;
    if (match({TokenType::Semicolon})) initializer = nullptr;
    else if (match({TokenType::Let, TokenType::Const})) initializer = varDeclaration();
    else initializer = expressionStatement();

    std::unique_ptr<ExprNode> condition = nullptr;
    if (!check(TokenType::Semicolon)) condition = expression();
    consume(TokenType::Semicolon, "Expected ';' after loop condition.");

    std::unique_ptr<ExprNode> increment = nullptr;
    if (!check(TokenType::RParen)) increment = expression();
    consume(TokenType::RParen, "Expected ')' after for clauses.");

    auto body = statement();
    return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment), std::move(body));
}

std::unique_ptr<StmtNode> Parser::switchStatement() {
    consume(TokenType::LParen, "Expected '(' after 'switch'.");
    auto condition = expression();
    consume(TokenType::RParen, "Expected ')' after switch condition.");
    consume(TokenType::LBrace, "Expected '{' after switch block.");

    std::vector<SwitchStmt::Case> cases;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        std::unique_ptr<ExprNode> caseValue = nullptr;
        if (match({TokenType::Case})) {
            caseValue = expression();
            consume(TokenType::Colon, "Expected ':' after case value.");
        } else if (match({TokenType::Default})) {
            consume(TokenType::Colon, "Expected ':' after default.");
        } else {
            throw error(peek(), "Expected 'case' or 'default' inside switch.");
        }

        std::vector<std::unique_ptr<StmtNode>> statements;
        while (!check(TokenType::Case) && !check(TokenType::Default) && !check(TokenType::RBrace) && !isAtEnd()) {
            statements.push_back(statement());
        }
        cases.push_back({std::move(caseValue), std::move(statements)});
    }
    consume(TokenType::RBrace, "Expected '}' to close switch block.");
    return std::make_unique<SwitchStmt>(std::move(condition), std::move(cases));
}

std::unique_ptr<StmtNode> Parser::returnStatement() {
    std::unique_ptr<ExprNode> value = nullptr;
    if (!check(TokenType::Semicolon)) value = expression();
    consume(TokenType::Semicolon, "Expected ';' after return value.");
    return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<StmtNode> Parser::breakStatement() {
    Token keyword = previous();
    consume(TokenType::Semicolon, "Expected ';' after 'break'.");
    return std::make_unique<BreakStmt>(keyword);
}

std::unique_ptr<StmtNode> Parser::continueStatement() {
    Token keyword = previous();
    consume(TokenType::Semicolon, "Expected ';' after 'continue'.");
    return std::make_unique<ContinueStmt>(keyword);
}

std::unique_ptr<BlockStmt> Parser::block() {
    std::vector<std::unique_ptr<StmtNode>> statements;
    while (!check(TokenType::RBrace) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    consume(TokenType::RBrace, "Expected '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<StmtNode> Parser::expressionStatement() {
    auto expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after expression.");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// ==========================================
// 4. EXPRESSIONS (Precedence Climbing)
// ==========================================

std::unique_ptr<ExprNode> Parser::expression() {
    return assignment();
}

std::unique_ptr<ExprNode> Parser::assignment() {
    auto expr = logicalOr();

    if (match({TokenType::Assign, TokenType::PlusAssign, TokenType::MinusAssign, 
               TokenType::StarAssign, TokenType::SlashAssign, TokenType::ModuloAssign, TokenType::ExponentAssign})) {
        Token equals = previous();
        auto value = assignment(); // Recursively parse the right side
        return std::make_unique<AssignmentExpr>(std::move(expr), equals, std::move(value));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::logicalOr() {
    auto expr = logicalAnd();
    while (match({TokenType::LogicalOr})) {
        Token op = previous();
        auto right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::logicalAnd() {
    auto expr = bitwiseOr();
    while (match({TokenType::LogicalAnd})) {
        Token op = previous();
        auto right = bitwiseOr();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseOr() {
    auto expr = bitwiseXor();
    while (match({TokenType::BitOr})) {
        Token op = previous();
        auto right = bitwiseXor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseXor() {
    auto expr = bitwiseAnd();
    while (match({TokenType::BitXor})) {
        Token op = previous();
        auto right = bitwiseAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::bitwiseAnd() {
    auto expr = equality();
    while (match({TokenType::BitAnd})) {
        Token op = previous();
        auto right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::equality() {
    auto expr = comparison();
    while (match({TokenType::Eq, TokenType::StrictEq, TokenType::NotEq, TokenType::StrictNotEq})) {
        Token op = previous();
        auto right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::comparison() {
    auto expr = shift();
    while (match({TokenType::Less, TokenType::Greater, TokenType::LessEq, TokenType::GreaterEq, TokenType::In})) {
        Token op = previous();
        auto right = shift();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::shift() {
    auto expr = term();
    while (match({TokenType::ShiftLeft, TokenType::ShiftRight, TokenType::UnsignedShiftRight})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::term() {
    auto expr = factor();
    while (match({TokenType::Plus, TokenType::Minus})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::factor() {
    auto expr = unary();
    while (match({TokenType::Star, TokenType::Slash, TokenType::Modulo, TokenType::Exponent})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<ExprNode> Parser::unary() {
    if (match({TokenType::New})) {
        auto expr = call();
        if (auto callExpr = dynamic_cast<CallExpr*>(expr.get())) {
            auto callee = std::move(callExpr->callee);
            auto args = std::move(callExpr->arguments);
            return std::make_unique<NewExpr>(std::move(callee), std::move(args));
        } else {
            return std::make_unique<NewExpr>(std::move(expr), std::vector<std::unique_ptr<ExprNode>>{});
        }
    }
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UpdateExpr>(std::move(right), op, true);
    }
    if (match({TokenType::LogicalNot, TokenType::Minus, TokenType::Typeof, TokenType::BitNot})) {
        Token op = previous();
        auto right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return call();
}

std::unique_ptr<ExprNode> Parser::call() {
    auto expr = primary();

    while (true) {
        if (match({TokenType::LParen})) { // Function call: myFunc(...)
            std::vector<std::unique_ptr<ExprNode>> args;
            if (!check(TokenType::RParen)) {
                do {
                    if (match({TokenType::Spread})) {
                        args.push_back(std::make_unique<SpreadExpr>(expression()));
                    } else {
                        args.push_back(expression());
                    }
                } while (match({TokenType::Comma}));
            }
            consume(TokenType::RParen, "Expected ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), std::move(args));
        } 
        else if (match({TokenType::Dot})) { // Property access: obj.name
            Token name = consume(TokenType::Identifier, "Expected property name after '.'.");
            expr = std::make_unique<MemberExpr>(std::move(expr), name, false);
        } 
else if (match({TokenType::LBracket})) { // Array index access: arr[i + 1]
            auto index = expression();
            consume(TokenType::RBracket, "Expected ']' after index.");
            // FIXED: Now we pass the full 'index' expression tree into the AST
            expr = std::make_unique<MemberExpr>(std::move(expr), std::move(index), true); 
        }
        else {
            break;
        }
    }
    
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        return std::make_unique<UpdateExpr>(std::move(expr), previous(), false);
    }
    
    return expr;
}

std::unique_ptr<ExprNode> Parser::primary() {
    if (match({TokenType::False})) return std::make_unique<LiteralExpr>(previous());
    if (match({TokenType::True})) return std::make_unique<LiteralExpr>(previous());
    if (match({TokenType::Null})) return std::make_unique<LiteralExpr>(previous());
    if (match({TokenType::Undefined})) return std::make_unique<LiteralExpr>(previous());
    
    if (match({TokenType::Number, TokenType::String})) {
        return std::make_unique<LiteralExpr>(previous());
    }

    // Single parameter arrow function without parens: a => ...
    if (check(TokenType::Identifier) && current + 1 < tokens.size() && tokens[current + 1].type == TokenType::Arrow) {
        Token param = advance(); // Consume identifier
        advance(); // Consume Arrow
        
        std::vector<Token> parameters = {param};
        std::unique_ptr<BlockStmt> body;
        if (match({TokenType::LBrace})) {
            body = block();
        } else {
            auto expr = expression();
            std::vector<std::unique_ptr<StmtNode>> stmts;
            stmts.push_back(std::make_unique<ReturnStmt>(std::move(expr)));
            body = std::make_unique<BlockStmt>(std::move(stmts));
        }
        return std::make_unique<FunctionExpr>(std::move(parameters), false, Token{}, std::move(body), true);
    }
    
    if (match({TokenType::This})) {
        return std::make_unique<ThisExpr>(previous());
    }

    if (match({TokenType::Identifier})) {
        return std::make_unique<IdentifierExpr>(previous());
    }

    if (match({TokenType::LBracket})) return arrayLiteral();
    if (match({TokenType::LBrace})) return objectLiteral();
    
    // Function Expressions (e.g., callbacks in .map())
    if (match({TokenType::Function})) {
        consume(TokenType::LParen, "Expected '(' after function.");
        std::vector<Token> parameters;
        bool hasRest = false;
        Token restParam;
        if (!check(TokenType::RParen)) {
            do {
                if (match({TokenType::Spread})) {
                    hasRest = true;
                    restParam = consume(TokenType::Identifier, "Expected parameter name after '...'.");
                    break;
                }
                parameters.push_back(consume(TokenType::Identifier, "Expected parameter name."));
            } while (match({TokenType::Comma}));
        }
        consume(TokenType::RParen, "Expected ')' after parameters.");
        consume(TokenType::LBrace, "Expected '{' before function body.");
        auto body = block();
        return std::make_unique<FunctionExpr>(std::move(parameters), hasRest, restParam, std::move(body));
    }

    // Grouping `( ... )` or Arrow Functions `(a, b) => { ... }` or `() => { ... }`
    if (match({TokenType::LParen})) {
        if (isArrowFunction() || (check(TokenType::RParen) && tokens[current + 1].type == TokenType::Arrow)) {
            // It's an arrow function
            std::vector<Token> parameters;
            bool hasRest = false;
            Token restParam;
            if (!check(TokenType::RParen)) {
                do {
                    if (match({TokenType::Spread})) {
                        hasRest = true;
                        restParam = consume(TokenType::Identifier, "Expected parameter name after '...'.");
                        break;
                    }
                    parameters.push_back(consume(TokenType::Identifier, "Expected parameter name."));
                } while (match({TokenType::Comma}));
            }
            consume(TokenType::RParen, "Expected ')' after parameters.");
            consume(TokenType::Arrow, "Expected '=>'.");
            
            std::unique_ptr<BlockStmt> body;
            if (match({TokenType::LBrace})) {
                body = block();
            } else {
                auto expr = expression();
                std::vector<std::unique_ptr<StmtNode>> stmts;
                stmts.push_back(std::make_unique<ReturnStmt>(std::move(expr)));
                body = std::make_unique<BlockStmt>(std::move(stmts));
            }
            return std::make_unique<FunctionExpr>(std::move(parameters), hasRest, restParam, std::move(body), true);
        }
        
        auto expr = expression();
        consume(TokenType::RParen, "Expected ')' after expression.");
        return expr;
    }

    throw error(peek(), "Expected expression.");
}

// ==========================================
// 5. LITERAL HELPERS
// ==========================================

std::unique_ptr<ExprNode> Parser::arrayLiteral() {
    std::vector<std::unique_ptr<ExprNode>> elements;
    if (!check(TokenType::RBracket)) {
        do {
            if (check(TokenType::RBracket)) break; // trailing comma
            if (match({TokenType::Spread})) {
                elements.push_back(std::make_unique<SpreadExpr>(expression()));
            } else if (check(TokenType::Comma)) {
                // Elision
                elements.push_back(std::make_unique<LiteralExpr>(Token{TokenType::Undefined, "undefined", peek().line, peek().column}));
            } else {
                elements.push_back(expression());
            }
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RBracket, "Expected ']' after array elements.");
    return std::make_unique<ArrayLiteralExpr>(std::move(elements));
}

std::unique_ptr<ExprNode> Parser::objectLiteral() {
    std::vector<ObjectProperty> properties;
    if (!check(TokenType::RBrace)) {
        do {
            if (check(TokenType::RBrace)) break; // trailing comma
            
            if (match({TokenType::Spread})) {
                properties.push_back({"", nullptr, false, true, false, false, std::make_unique<SpreadExpr>(expression())});
                continue;
            }
            
            bool isGetter = false;
            bool isSetter = false;
            bool isComputed = false;
            std::unique_ptr<ExprNode> computedKey = nullptr;
            std::string keyName = "";
            Token keyToken;

            // Check if it's a computed property
            if (match({TokenType::LBracket})) {
                isComputed = true;
                computedKey = expression();
                consume(TokenType::RBracket, "Expected ']' after computed property key.");
            } else if (match({TokenType::Identifier, TokenType::String})) {
                keyToken = previous();
                keyName = keyToken.lexeme;
                if (keyToken.type == TokenType::String && keyName.length() >= 2) {
                    keyName = keyName.substr(1, keyName.length() - 2);
                }

                // Check for getter/setter
                if (keyToken.type == TokenType::Identifier && (keyName == "get" || keyName == "set") && !check(TokenType::Colon) && !check(TokenType::LParen) && !check(TokenType::Comma) && !check(TokenType::RBrace)) {
                    if (keyName == "get") isGetter = true;
                    else isSetter = true;

                    if (match({TokenType::LBracket})) {
                        isComputed = true;
                        computedKey = expression();
                        consume(TokenType::RBracket, "Expected ']' after computed property key.");
                    } else if (match({TokenType::Identifier, TokenType::String})) {
                        keyToken = previous();
                        keyName = keyToken.lexeme;
                        if (keyToken.type == TokenType::String && keyName.length() >= 2) {
                            keyName = keyName.substr(1, keyName.length() - 2);
                        }
                    } else {
                        throw error(peek(), "Expected property name after get/set.");
                    }
                }
            } else {
                throw error(peek(), "Expected property name.");
            }

            // Check for method shorthand
            if (match({TokenType::LParen})) {
                std::vector<Token> parameters;
                bool hasRest = false;
                Token restParam;
                if (!check(TokenType::RParen)) {
                    do {
                        if (match({TokenType::Spread})) {
                            hasRest = true;
                            restParam = consume(TokenType::Identifier, "Expected parameter name after '...'.");
                            break;
                        }
                        parameters.push_back(consume(TokenType::Identifier, "Expected parameter name."));
                    } while (match({TokenType::Comma}));
                }
                consume(TokenType::RParen, "Expected ')' after parameters.");
                consume(TokenType::LBrace, "Expected '{' before method body.");
                auto body = block();
                auto funcExpr = std::make_unique<FunctionExpr>(std::move(parameters), hasRest, restParam, std::move(body), false);
                properties.push_back({keyName, std::move(computedKey), isComputed, false, isGetter, isSetter, std::move(funcExpr)});
                continue;
            }
            
            // Check for shorthand object property
            if (!isComputed && !isGetter && !isSetter && (check(TokenType::Comma) || check(TokenType::RBrace))) {
                if (keyToken.type == TokenType::String) throw error(peek(), "String literal cannot be used as shorthand.");
                properties.push_back({keyName, nullptr, false, false, false, false, std::make_unique<IdentifierExpr>(keyToken)});
                continue;
            }
            
            consume(TokenType::Colon, "Expected ':' after property name.");
            auto valueExpr = expression();
            
            properties.push_back({keyName, std::move(computedKey), isComputed, false, isGetter, isSetter, std::move(valueExpr)});
        } while (match({TokenType::Comma}));
    }
    consume(TokenType::RBrace, "Expected '}' after object properties.");
    return std::make_unique<ObjectLiteralExpr>(std::move(properties));
}