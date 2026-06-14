#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// ==========================================
// 1. TOKEN TYPES
// ==========================================
enum class TokenType {
    // Keywords
    Let, Const,    Function,
    Return,
    If,
    Else,
    For,
    While,
    Do,
    Switch,
    Case,
    Default,
    Break,
    Continue,
    This,
    In,
    New,
    Typeof,
    Identifier, Number, String, True, False, Null, Undefined,

    // Arithmetic & Assignment Operators
    Plus, Minus, Star, Slash, Modulo, Exponent,       // + - * / % **
    Assign, PlusAssign, MinusAssign,                  // = += -= 
    StarAssign, SlashAssign, ModuloAssign, ExponentAssign, // *= /= %= **=
    PlusPlus, MinusMinus,                             // ++ --
    BitAnd, BitOr, BitXor, BitNot, ShiftLeft, ShiftRight, UnsignedShiftRight, // & | ^ ~ << >> >>>

    // Comparison & Logical Operators
    Eq, StrictEq, NotEq, StrictNotEq,                 // == === != !==
    Less, LessEq, Greater, GreaterEq,                 // < <= > >=
    LogicalAnd, LogicalOr, LogicalNot,                // && || !

    // Punctuation & Structural
    LParen, RParen,                                   // ( )
    LBrace, RBrace,                                   // { }
    LBracket, RBracket,                               // [ ]
    Comma, Dot, Semicolon, Colon,                     // , . ; :
    Arrow, Spread,                                    // => ...

    // Special

    // End of file
    EndOfFile,
    Illegal // Used if the user types an invalid JS character (like @)
};

// ==========================================
// 2. THE TOKEN STRUCT
// ==========================================
// A Token holds what type it is, the exact string it represents (lexeme),
// and exactly where it was found in the file so we can print helpful error messages.
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    // Helper for debugging: Prints [Type: lexeme]
    std::string toString() const;
};

// ==========================================
// 3. THE LEXER CLASS
// ==========================================
class Lexer {
private:
    std::string source;
    size_t start = 0;   // Start of the current lexeme being scanned
    size_t current = 0; // Current character being looked at
    int line = 1;
    int column = 1;

    // A map to instantly identify if a word is a variable name or a reserved keyword
    std::unordered_map<std::string, TokenType> keywords;

    // --- Core Helper Methods ---
    bool isAtEnd() const;
    char advance();                // Consume the next character and return it
    char peek() const;             // Look at the current character without consuming it
    char peekNext() const;         // Look one character ahead
    bool match(char expected);     // Consume the character ONLY if it matches what we expect

    // --- Specific Scanners ---
    void scanToken(std::vector<Token>& tokens);
    void scanString(std::vector<Token>& tokens, char quoteType);
    void scanNumber(std::vector<Token>& tokens);
    void scanIdentifier(std::vector<Token>& tokens);
    
    // --- Token Emitter ---
    void addToken(std::vector<Token>& tokens, TokenType type);

public:
    Lexer(const std::string& sourceCode);
    
    // The main engine loop: converts the source string into the final token array
    std::vector<Token> tokenize();
};