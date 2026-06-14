#pragma once
#include "ast_nodes.h"
#include <string>
#include <sstream>
#include <vector>
#include <memory>

class Generator : public ASTVisitor {
private:
    std::ostringstream out; // Accumulates the final C++ code
    int indentLevel = 0;    // Tracks indentation for pretty-printing

    // --- Helper Methods ---
    
    // Prints the correct number of spaces based on indentLevel
    void writeIndent();
    
    // Checks if an expression needs to be wrapped in a JSValue() constructor
    // (Used to seamlessly connect raw C++ strings/numbers to our JSValue library)
    bool isStandardLibraryNode(ASTNode* node);

    // Generates a vector of JSValues, handling SpreadExpr dynamically
    void generateArguments(const std::vector<std::unique_ptr<ExprNode>>& arguments);

public:
    Generator() = default;

    // The main entry point: takes the AST and returns a massive string of valid C++ code
    std::string generate(const std::vector<std::unique_ptr<StmtNode>>& statements);

    // ==========================================
    // 1. EXPRESSION VISITORS (Outputting inline values)
    // ==========================================
    void visit(LiteralExpr* node) override;
    void visit(IdentifierExpr* node) override;
    void visit(ThisExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(UpdateExpr* node) override;
    void visit(AssignmentExpr* node) override;
    void visit(CallExpr* node) override;
    void visit(NewExpr* node) override;
    void visit(MemberExpr* node) override;
    void visit(ArrayLiteralExpr* node) override;
    void visit(ObjectLiteralExpr* node) override;
    void visit(SpreadExpr* node) override;
    void visit(FunctionExpr* node) override;

    // ==========================================
    // 2. STATEMENT VISITORS (Outputting logic and control flow)
    // ==========================================
    void visit(BlockStmt* node) override;
    void visit(ExpressionStmt* node) override;
    void visit(VariableDeclStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(WhileStmt* node) override;
    void visit(DoWhileStmt* node) override;
    void visit(ForStmt* node) override;
    void visit(SwitchStmt* node) override;
    void visit(FunctionDeclStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(BreakStmt* node) override;
    void visit(ContinueStmt* node) override;
};