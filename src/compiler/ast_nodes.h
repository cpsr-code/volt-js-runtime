#pragma once
#include <string>
#include <vector>
#include <memory>
#include "lexer.h"

// ==========================================
// FORWARD DECLARATIONS
// ==========================================
// Tells the compiler these classes exist before they are fully defined below.
class ASTVisitor;
struct BlockStmt; 

// ==========================================
// 1. BASE AST NODES
// ==========================================

struct ASTNode {
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

struct ExprNode : public ASTNode {};
struct StmtNode : public ASTNode {};

// FIX: We only declare the function here. The implementation is at the bottom!
#define ACCEPT_VISITOR \
    void accept(ASTVisitor& visitor) override;

// ==========================================
// 2. EXPRESSION NODES
// ==========================================

struct LiteralExpr : public ExprNode {
    Token value; 
    LiteralExpr(Token val) : value(val) {}
    ACCEPT_VISITOR
};

struct IdentifierExpr : public ExprNode {
    Token name; 
    IdentifierExpr(Token n) : name(n) {}
    ACCEPT_VISITOR
};

struct ThisExpr : public ExprNode {
    Token keyword;
    ThisExpr(Token kw) : keyword(kw) {}
    ACCEPT_VISITOR
};

struct BinaryExpr : public ExprNode {
    std::unique_ptr<ExprNode> left;
    Token op; 
    std::unique_ptr<ExprNode> right;

    BinaryExpr(std::unique_ptr<ExprNode> l, Token o, std::unique_ptr<ExprNode> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    ACCEPT_VISITOR
};

struct UnaryExpr : public ExprNode {
    Token op; 
    std::unique_ptr<ExprNode> right;

    UnaryExpr(Token o, std::unique_ptr<ExprNode> r) : op(o), right(std::move(r)) {}
    ACCEPT_VISITOR
};

struct UpdateExpr : public ExprNode {
    std::unique_ptr<ExprNode> operand;
    Token op;
    bool isPrefix;

    UpdateExpr(std::unique_ptr<ExprNode> operand, Token op, bool isPrefix)
        : operand(std::move(operand)), op(op), isPrefix(isPrefix) {}
    ACCEPT_VISITOR
};

struct AssignmentExpr : public ExprNode {
    std::unique_ptr<ExprNode> target; 
    Token op; 
    std::unique_ptr<ExprNode> value;

    AssignmentExpr(std::unique_ptr<ExprNode> t, Token o, std::unique_ptr<ExprNode> v)
        : target(std::move(t)), op(o), value(std::move(v)) {}
    ACCEPT_VISITOR
};

struct CallExpr : public ExprNode {
    std::unique_ptr<ExprNode> callee; 
    std::vector<std::unique_ptr<ExprNode>> arguments; 

    CallExpr(std::unique_ptr<ExprNode> c, std::vector<std::unique_ptr<ExprNode>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
    ACCEPT_VISITOR
};

struct NewExpr : public ExprNode {
    std::unique_ptr<ExprNode> callee; 
    std::vector<std::unique_ptr<ExprNode>> arguments; 

    NewExpr(std::unique_ptr<ExprNode> c, std::vector<std::unique_ptr<ExprNode>> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
    ACCEPT_VISITOR
};

struct MemberExpr : public ExprNode {
    std::unique_ptr<ExprNode> object; 
    Token property; 
    std::unique_ptr<ExprNode> propertyExpr; 
    bool computed; 

    // Constructor 1: For standard dot notation (computed = false)
    MemberExpr(std::unique_ptr<ExprNode> obj, Token prop, bool comp)
        : object(std::move(obj)), property(prop), propertyExpr(nullptr), computed(comp) {}

    // Constructor 2: For computed bracket notation (computed = true)
    MemberExpr(std::unique_ptr<ExprNode> obj, std::unique_ptr<ExprNode> propExpr, bool comp)
        : object(std::move(obj)), property(Token{}), propertyExpr(std::move(propExpr)), computed(comp) {}

    ACCEPT_VISITOR
};

struct SpreadExpr : public ExprNode {
    std::unique_ptr<ExprNode> expression;
    SpreadExpr(std::unique_ptr<ExprNode> e) : expression(std::move(e)) {}
    ACCEPT_VISITOR
};

struct ArrayLiteralExpr : public ExprNode {
    std::vector<std::unique_ptr<ExprNode>> elements;
    ArrayLiteralExpr(std::vector<std::unique_ptr<ExprNode>> elems) : elements(std::move(elems)) {}
    ACCEPT_VISITOR
};

struct ObjectProperty {
    std::string key;
    std::unique_ptr<ExprNode> computedKey;
    bool isComputed;
    bool isSpread;
    bool isGetter;
    bool isSetter;
    std::unique_ptr<ExprNode> value;
};

struct ObjectLiteralExpr : public ExprNode {
    std::vector<ObjectProperty> properties;
    ObjectLiteralExpr(std::vector<ObjectProperty> props)
        : properties(std::move(props)) {}
    ACCEPT_VISITOR
};

struct FunctionExpr : public ExprNode {
    std::vector<Token> params;
    bool hasRest;
    Token restParam;
    std::unique_ptr<BlockStmt> body;
    bool isArrow;
    FunctionExpr(std::vector<Token> p, bool hr, Token rp, std::unique_ptr<BlockStmt> b, bool arrow = false)
        : params(std::move(p)), hasRest(hr), restParam(rp), body(std::move(b)), isArrow(arrow) {}
    ACCEPT_VISITOR
};

// ==========================================
// 3. STATEMENT NODES
// ==========================================

struct BlockStmt : public StmtNode {
    std::vector<std::unique_ptr<StmtNode>> statements;
    BlockStmt(std::vector<std::unique_ptr<StmtNode>> stmts) : statements(std::move(stmts)) {}
    ACCEPT_VISITOR
};

struct ExpressionStmt : public StmtNode {
    std::unique_ptr<ExprNode> expression;
    ExpressionStmt(std::unique_ptr<ExprNode> expr) : expression(std::move(expr)) {}
    ACCEPT_VISITOR
};

struct VariableDeclStmt : public StmtNode {
    bool isConst;
    std::vector<std::pair<Token, std::unique_ptr<ExprNode>>> declarators;

    VariableDeclStmt(bool isC, std::vector<std::pair<Token, std::unique_ptr<ExprNode>>> decls)
        : isConst(isC), declarators(std::move(decls)) {}
    ACCEPT_VISITOR
};

struct IfStmt : public StmtNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> thenBranch;
    std::unique_ptr<StmtNode> elseBranch; 

    IfStmt(std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> thenB, std::unique_ptr<StmtNode> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    ACCEPT_VISITOR
};

struct WhileStmt : public StmtNode {
    std::unique_ptr<ExprNode> condition;
    std::unique_ptr<StmtNode> body;

    WhileStmt(std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    ACCEPT_VISITOR
};

struct DoWhileStmt : public StmtNode {
    std::unique_ptr<StmtNode> body;
    std::unique_ptr<ExprNode> condition;

    DoWhileStmt(std::unique_ptr<StmtNode> b, std::unique_ptr<ExprNode> cond)
        : body(std::move(b)), condition(std::move(cond)) {}
    ACCEPT_VISITOR
};

struct ForStmt : public StmtNode {
    std::unique_ptr<StmtNode> initializer; 
    std::unique_ptr<ExprNode> condition;   
    std::unique_ptr<ExprNode> increment;   
    std::unique_ptr<StmtNode> body;        

    ForStmt(std::unique_ptr<StmtNode> init, std::unique_ptr<ExprNode> cond, 
            std::unique_ptr<ExprNode> inc, std::unique_ptr<StmtNode> b)
        : initializer(std::move(init)), condition(std::move(cond)), 
          increment(std::move(inc)), body(std::move(b)) {}
    ACCEPT_VISITOR
};

struct SwitchStmt : public StmtNode {
    std::unique_ptr<ExprNode> condition;
    
    struct Case {
        std::unique_ptr<ExprNode> value; 
        std::vector<std::unique_ptr<StmtNode>> statements;
    };
    std::vector<Case> cases;

    SwitchStmt(std::unique_ptr<ExprNode> cond, std::vector<Case> c)
        : condition(std::move(cond)), cases(std::move(c)) {}
    ACCEPT_VISITOR
};

struct FunctionDeclStmt : public StmtNode {
    Token name;
    std::vector<Token> params;
    bool hasRest;
    Token restParam;
    std::unique_ptr<BlockStmt> body; 

    FunctionDeclStmt(Token n, std::vector<Token> p, bool hr, Token rp, std::unique_ptr<BlockStmt> b)
        : name(n), params(std::move(p)), hasRest(hr), restParam(rp), body(std::move(b)) {}
    ACCEPT_VISITOR
};

struct ReturnStmt : public StmtNode {
    std::unique_ptr<ExprNode> value; 
    ReturnStmt(std::unique_ptr<ExprNode> val) : value(std::move(val)) {}
    ACCEPT_VISITOR
};

struct BreakStmt : public StmtNode {
    Token keyword;
    BreakStmt(Token k) : keyword(k) {}
    ACCEPT_VISITOR
};

struct ContinueStmt : public StmtNode {
    Token keyword;
    ContinueStmt(Token k) : keyword(k) {}
    ACCEPT_VISITOR
};

// ========================================
// 4. THE VISITOR INTERFACE
// ==========================================

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(LiteralExpr* node) = 0;
    virtual void visit(IdentifierExpr* node) = 0;
    virtual void visit(ThisExpr* node) = 0;
    virtual void visit(BinaryExpr* node) = 0;
    virtual void visit(UnaryExpr* node) = 0;
    virtual void visit(UpdateExpr* node) = 0;
    virtual void visit(AssignmentExpr* node) = 0;
    virtual void visit(CallExpr* node) = 0;
    virtual void visit(NewExpr* node) = 0;
    virtual void visit(MemberExpr* node) = 0;
    virtual void visit(ArrayLiteralExpr* node) = 0;
    virtual void visit(ObjectLiteralExpr* node) = 0;
    virtual void visit(SpreadExpr* node) = 0;
    virtual void visit(FunctionExpr* node) = 0;

    virtual void visit(BlockStmt* node) = 0;
    virtual void visit(ExpressionStmt* node) = 0;
    virtual void visit(VariableDeclStmt* node) = 0;
    virtual void visit(IfStmt* node) = 0;
    virtual void visit(WhileStmt* node) = 0;
    virtual void visit(DoWhileStmt* node) = 0;
    virtual void visit(ForStmt* node) = 0;
    virtual void visit(SwitchStmt* node) = 0;
    virtual void visit(FunctionDeclStmt* node) = 0;
    virtual void visit(ReturnStmt* node) = 0;
    virtual void visit(BreakStmt* node) = 0;
    virtual void visit(ContinueStmt* node) = 0;
};

// ==========================================
// 5. VISITOR MACRO IMPLEMENTATIONS
// ==========================================
// FIX: These are now defined safely at the bottom after ASTVisitor is fully known.

inline void LiteralExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void IdentifierExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ThisExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void BinaryExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void UnaryExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void UpdateExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void AssignmentExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void CallExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void NewExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void MemberExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ArrayLiteralExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ObjectLiteralExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void SpreadExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void FunctionExpr::accept(ASTVisitor& visitor) { visitor.visit(this); }

inline void BlockStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ExpressionStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void VariableDeclStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void IfStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void WhileStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void DoWhileStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ForStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void SwitchStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void FunctionDeclStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ReturnStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void BreakStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }
inline void ContinueStmt::accept(ASTVisitor& visitor) { visitor.visit(this); }