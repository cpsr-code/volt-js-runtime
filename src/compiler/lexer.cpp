#include "lexer.h"
#include <cctype>
#include <iostream>

// ==========================================
// 1. TOKEN DEBUG HELPER
// ==========================================

std::string Token::toString() const {
    return "[Line " + std::to_string(line) + ":" + std::to_string(column) + 
           "] Lexeme: '" + lexeme + "'";
}

// ==========================================
// 2. CONSTRUCTOR & SETUP
// ==========================================

Lexer::Lexer(const std::string& sourceCode) : source(sourceCode) {
    // Register all reserved JavaScript keywords
    keywords["let"] = TokenType::Let;
    keywords["const"] = TokenType::Const;
    keywords["function"] = TokenType::Function;
    keywords["return"] = TokenType::Return;
    keywords["if"] = TokenType::If;
    keywords["else"] = TokenType::Else;
    keywords["for"] = TokenType::For;
    keywords["while"] = TokenType::While;
    keywords["do"] = TokenType::Do;
    keywords["switch"] = TokenType::Switch;
    keywords["case"] = TokenType::Case;
    keywords["default"] = TokenType::Default;
    keywords["new"] = TokenType::New;
    keywords["true"] = TokenType::True;
    keywords["false"] = TokenType::False;
    keywords["null"] = TokenType::Null;
    keywords["undefined"] = TokenType::Undefined;
    keywords["typeof"] = TokenType::Typeof;
    keywords["break"] = TokenType::Break;
    keywords["continue"] = TokenType::Continue;
    keywords["this"] = TokenType::This;
    keywords["in"] = TokenType::In;
}

// ==========================================
// 3. CORE HELPER METHODS
// ==========================================

bool Lexer::isAtEnd() const {
    return current >= source.length();
}

char Lexer::advance() {
    column++;
    return source[current++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::addToken(std::vector<Token>& tokens, TokenType type) {
    std::string lexeme = source.substr(start, current - start);
    // The starting column of this token is the current column minus its length
    int startColumn = column - (current - start);
    tokens.push_back({type, lexeme, line, startColumn});
}

// ==========================================
// 4. THE MAIN SCANNER LOOP
// ==========================================

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        start = current;
        scanToken(tokens);
    }

    // Always append an EndOfFile token to safely stop the Parser later
    tokens.push_back({TokenType::EndOfFile, "", line, column});
    return tokens;
}

void Lexer::scanToken(std::vector<Token>& tokens) {
    char c = advance();

    switch (c) {
        // --- 1. Single Character Punctuation ---
        case '(': addToken(tokens, TokenType::LParen); break;
        case ')': addToken(tokens, TokenType::RParen); break;
        case '{': addToken(tokens, TokenType::LBrace); break;
        case '}': addToken(tokens, TokenType::RBrace); break;
        case '[': addToken(tokens, TokenType::LBracket); break;
        case ']': addToken(tokens, TokenType::RBracket); break;
        case ',': addToken(tokens, TokenType::Comma); break;
        case ';': addToken(tokens, TokenType::Semicolon); break;
        case ':': addToken(tokens, TokenType::Colon); break;

        case '!': 
            addToken(tokens, match('=') ? (match('=') ? TokenType::StrictNotEq : TokenType::NotEq) : TokenType::LogicalNot); 
            break;
        case '%': addToken(tokens, match('=') ? TokenType::ModuloAssign : TokenType::Modulo); break;
        case '*': 
            if (match('=')) addToken(tokens, TokenType::StarAssign);
            else if (match('*')) addToken(tokens, match('=') ? TokenType::ExponentAssign : TokenType::Exponent);
            else addToken(tokens, TokenType::Star);
            break;
        case '~': addToken(tokens, TokenType::BitNot); break;
        case '^': addToken(tokens, TokenType::BitXor); break;
        case '&': addToken(tokens, match('&') ? TokenType::LogicalAnd : TokenType::BitAnd); break;
        case '|': addToken(tokens, match('|') ? TokenType::LogicalOr : TokenType::BitOr); break;
        
        case '<': 
            if (match('=')) addToken(tokens, TokenType::LessEq);
            else if (match('<')) addToken(tokens, TokenType::ShiftLeft);
            else addToken(tokens, TokenType::Less); 
            break;
            
        case '>': 
            if (match('=')) addToken(tokens, TokenType::GreaterEq);
            else if (match('>')) {
                if (match('>')) addToken(tokens, TokenType::UnsignedShiftRight);
                else addToken(tokens, TokenType::ShiftRight);
            }
            else addToken(tokens, TokenType::Greater); 
            break;

        // --- 2. Tricky Operators (Requires Lookahead) ---
        case '.':
            if (peek() == '.' && peekNext() == '.') { // Spread Operator: ...
                advance(); advance();
                addToken(tokens, TokenType::Spread);
            } else {
                addToken(tokens, TokenType::Dot);
            }
            break;
            
        case '+':
            if (match('=')) addToken(tokens, TokenType::PlusAssign);
            else if (match('+')) addToken(tokens, TokenType::PlusPlus);
            else addToken(tokens, TokenType::Plus);
            break;
            
        case '-':
            if (match('=')) addToken(tokens, TokenType::MinusAssign);
            else if (match('-')) addToken(tokens, TokenType::MinusMinus);
            else addToken(tokens, TokenType::Minus);
            break;
            
        case '=':
            if (match('=')) {
                if (match('=')) addToken(tokens, TokenType::StrictEq); // ===
                else addToken(tokens, TokenType::Eq);                  // ==
            } else if (match('>')) {
                addToken(tokens, TokenType::Arrow);                    // =>
            } else {
                addToken(tokens, TokenType::Assign);                   // =
            }
            break;
            


        // --- 3. Comments and Division ---
        case '/':
            if (match('/')) {
                // Single-line comment: keep going until the end of the line
                while (peek() != '\n' && !isAtEnd()) advance();
            } else if (match('*')) {
                // Multi-line comment: keep going until */
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); advance(); // Consume */
                        break;
                    }
                    if (peek() == '\n') {
                        line++;
                        column = 0; // Reset column on new line
                    }
                    advance();
                }
            } else if (match('=')) {
                addToken(tokens, TokenType::SlashAssign);
            } else {
                addToken(tokens, TokenType::Slash);
            }
            break;

        // --- 4. Whitespace (Ignore) ---
        case ' ':
        case '\r':
        case '\t':
            break; // Do nothing
        case '\n':
            line++;
            column = 0; // Reset column tracker on new line
            break;

        // --- 5. Literals (Strings, Numbers, Identifiers) ---
        case '"':
        case '\'':
        case '`':
            scanString(tokens, c); // Pass the quote type so we match it
            break;

        default:
            if (std::isdigit(c)) {
                scanNumber(tokens);
            } else if (std::isalpha(c) || c == '_') {
                scanIdentifier(tokens);
            } else {
                addToken(tokens, TokenType::Illegal);
            }
            break;
    }
}

// ==========================================
// 5. SPECIFIC LITERAL SCANNERS
// ==========================================

void Lexer::scanString(std::vector<Token>& tokens, char quoteType) {
    while (peek() != quoteType && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 0;
        }
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Error: Unterminated string at line " << line << std::endl;
        return;
    }

    // Consume the closing quote
    advance();
    addToken(tokens, TokenType::String);
}

void Lexer::scanNumber(std::vector<Token>& tokens) {
    while (std::isdigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance(); // Consume the "."
        while (std::isdigit(peek())) advance();
    }

    addToken(tokens, TokenType::Number);
}

void Lexer::scanIdentifier(std::vector<Token>& tokens) {
    // Identifiers can be alphanumeric or underscores
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }

    std::string text = source.substr(start, current - start);
    
    // Check if the identifier is actually a reserved keyword
    if (keywords.find(text) != keywords.end()) {
        addToken(tokens, keywords[text]);
    } else {
        addToken(tokens, TokenType::Identifier);
    }
}