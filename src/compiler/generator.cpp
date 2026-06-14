#include "generator.h"
#include <iostream>

// ==========================================
// CORE HELPERS
// ==========================================

void Generator::writeIndent() {
    out << std::string(indentLevel * 4, ' ');
}

void Generator::generateArguments(const std::vector<std::unique_ptr<ExprNode>>& arguments) {
    bool hasSpread = false;
    for (const auto& arg : arguments) {
        if (dynamic_cast<SpreadExpr*>(arg.get())) hasSpread = true;
    }

    if (!hasSpread) {
        out << "{";
        for (size_t i = 0; i < arguments.size(); ++i) {
            arguments[i]->accept(*this);
            if (i < arguments.size() - 1) out << ", ";
        }
        out << "}";
    } else {
        out << "([&]() -> std::vector<JSValue> { std::vector<JSValue> _args; ";
        int spreadCounter = 0;
        for (const auto& arg : arguments) {
            if (auto spread = dynamic_cast<SpreadExpr*>(arg.get())) {
                out << "{ auto _spread" << spreadCounter << " = ";
                spread->expression->accept(*this);
                out << "; if (_spread" << spreadCounter << ".isArray()) { auto _arr = std::get<std::shared_ptr<JSArray>>(_spread" << spreadCounter << ".value); for (size_t i = 0; i < _arr->length(); ++i) _args.push_back((*_arr)[i]); } } ";
                spreadCounter++;
            } else {
                out << "_args.push_back(";
                arg->accept(*this);
                out << "); ";
            }
        }
        out << "return _args; })()";
    }
}

std::string Generator::generate(const std::vector<std::unique_ptr<StmtNode>>& statements) {
// Inject the required headers from Phase 1
    // Paths are now relative to the project root
    out << "#include \"src/lib/js_value.h\"\n";
    out << "#include \"src/lib/js_array.h\"\n";
    out << "#include \"src/lib/js_object.h\"\n";
    out << "#include \"src/lib/js_function.h\"\n";
    out << "#include \"src/lib/js_math.h\"\n";
    out << "#include \"src/lib/js_date.h\"\n\n";
    
    out << "int main() {\n";
    indentLevel++;
    
    // Hoist functions first
    for (const auto& stmt : statements) {
        if (auto funcDecl = dynamic_cast<FunctionDeclStmt*>(stmt.get())) {
            funcDecl->accept(*this);
        }
    }
    
    // Visit other statements
    for (const auto& stmt : statements) {
        if (stmt && !dynamic_cast<FunctionDeclStmt*>(stmt.get())) {
            stmt->accept(*this);
        }
    }
    
    indentLevel--;
    out << "    return 0;\n";
    out << "}\n";
    
    return out.str();
}

// ==========================================
// 1. STATEMENT VISITORS
// ==========================================

void Generator::visit(VariableDeclStmt* node) {
    for (const auto& pair : node->declarators) {
        writeIndent();
        if (node->isConst) out << "const ";
        out << "JSValue _v_" << pair.first.lexeme; 
        
        if (pair.second) {
            out << " = ";
            pair.second->accept(*this);
        }
        out << ";\n";
    }
}

void Generator::visit(ExpressionStmt* node) {
    writeIndent();
    node->expression->accept(*this);
    out << ";\n";
}

void Generator::visit(BlockStmt* node) {
    out << "{\n";
    indentLevel++;
    
    // Hoist functions
    for (const auto& stmt : node->statements) {
        if (auto funcDecl = dynamic_cast<FunctionDeclStmt*>(stmt.get())) {
            funcDecl->accept(*this);
        }
    }
    
    // Process other statements
    for (const auto& stmt : node->statements) {
        if (!dynamic_cast<FunctionDeclStmt*>(stmt.get())) {
            stmt->accept(*this);
        }
    }
    
    indentLevel--;
    writeIndent();
    out << "}\n";
}

void Generator::visit(IfStmt* node) {
    writeIndent();
    out << "if ((";
    node->condition->accept(*this);
    out << ").toBoolean()) "; // Use Phase 1 coercion logic!
    
    node->thenBranch->accept(*this);
    
    if (node->elseBranch) {
        writeIndent();
        out << "else ";
        node->elseBranch->accept(*this);
    }
}

void Generator::visit(WhileStmt* node) {
    writeIndent();
    out << "while ((";
    node->condition->accept(*this);
    out << ").toBoolean()) ";
    node->body->accept(*this);
}

void Generator::visit(DoWhileStmt* node) {
    writeIndent();
    out << "do ";
    node->body->accept(*this);
    writeIndent();
    out << "while ((";
    node->condition->accept(*this);
    out << ").toBoolean());\n";
}

void Generator::visit(ForStmt* node) {
    writeIndent();
    out << "{\n"; // Wrap in a block to scope the initializer variable (like 'let i = 0')
    indentLevel++;
    
    if (node->initializer) node->initializer->accept(*this);
    
    writeIndent();
    out << "for (; ";
    
    if (node->condition) {
        out << "(";
        node->condition->accept(*this);
        out << ").toBoolean()";
    } else {
        out << "true";
    }
    out << "; ";
    
    if (node->increment) node->increment->accept(*this);
    out << ") ";
    
    node->body->accept(*this);
    
    indentLevel--;
    writeIndent();
    out << "}\n";
}

void Generator::visit(SwitchStmt* node) {
    writeIndent();
    out << "{\n";
    indentLevel++;
    
    writeIndent();
    out << "JSValue _switch_val = ";
    node->condition->accept(*this);
    out << ";\n";
    
    writeIndent();
    out << "bool _fallthrough = false;\n";
    
    writeIndent();
    out << "do {\n";
    indentLevel++;
    
    for (const auto& c : node->cases) {
        writeIndent();
        if (c.value) {
            out << "if (_fallthrough || _switch_val.strictEquals(";
            c.value->accept(*this);
            out << ")) {\n";
        } else {
            out << "if (true) {\n"; // default case
        }
        
        indentLevel++;
        writeIndent();
        out << "_fallthrough = true;\n";
        
        for (const auto& stmt : c.statements) {
            stmt->accept(*this);
        }
        
        indentLevel--;
        writeIndent();
        out << "}\n";
    }
    
    indentLevel--;
    writeIndent();
    out << "} while(0);\n";
    
    indentLevel--;
    writeIndent();
    out << "}\n";
}

void Generator::visit(FunctionDeclStmt* node) {
    out << "JSValue _v_" << node->name.lexeme << " = JSValue(JSFunction::create([&](const std::vector<JSValue>& args, JSValue _thisContext) -> JSValue {\n";
    indentLevel++;
    
    // Safely map incoming arguments to JS variables
    for (size_t i = 0; i < node->params.size(); ++i) {
        writeIndent();
        out << "JSValue _v_" << node->params[i].lexeme << " = args.size() > " << i 
            << " ? args[" << i << "] : JSValue();\n";
    }
    
    if (node->hasRest) {
        writeIndent();
        out << "auto _restArr = JSArray::create({});\n";
        writeIndent();
        out << "for (size_t i = " << node->params.size() << "; i < args.size(); ++i) _restArr->push({args[i]});\n";
        writeIndent();
        out << "JSValue _v_" << node->restParam.lexeme << " = JSValue(_restArr);\n";
    }
    
    for (const auto& stmt : node->body->statements) {
        stmt->accept(*this);
    }
    
    // Implicit return for JS functions that lack a return statement
    writeIndent();
    out << "return JSValue();\n"; 
    
    indentLevel--;
    writeIndent();
    out << "}));\n";
}

void Generator::visit(ReturnStmt* node) {
    writeIndent();
    out << "return ";
    if (node->value) {
        node->value->accept(*this);
    } else {
        out << "JSValue()"; // return undefined;
    }
    out << ";\n";
}

void Generator::visit(BreakStmt* node) {
    writeIndent();
    out << "break;\n";
}

void Generator::visit(ContinueStmt* node) {
    writeIndent();
    out << "continue;\n";
}

// ==========================================
// 2. EXPRESSION VISITORS
// ==========================================

void Generator::visit(LiteralExpr* node) {
    if (node->value.type == TokenType::Number) out << "JSValue(" << node->value.lexeme << ")";
    else if (node->value.type == TokenType::String) {
        std::string raw = node->value.lexeme;
        out << "JSValue(\"";
        for (size_t i = 1; i < raw.length() - 1; ++i) {
            if (raw[i] == '"' && (i == 1 || raw[i-1] != '\\')) out << "\\\"";
            else if (raw[i] == '\\' && i + 1 < raw.length() - 1 && raw[i+1] == '\'') {
                out << "'";
                i++;
            }
            else out << raw[i];
        }
        out << "\")";
    }
    else if (node->value.type == TokenType::True) out << "JSValue(true)";
    else if (node->value.type == TokenType::False) out << "JSValue(false)";
    else if (node->value.type == TokenType::Null) out << "JSValue(JSNull{})";
    else if (node->value.type == TokenType::Undefined) out << "JSValue(JSUndefined{})";
}

void Generator::visit(IdentifierExpr* node) {
    std::string name = node->name.lexeme;
    if (name == "NaN") {
        out << "JSValue(std::nan(\"\"))";
    } else if (name == "Infinity") {
        out << "JSValue(INFINITY)";
    } else if (name == "console" || name == "Math" || name == "Date") {
        out << name;
    } else {
        out << "_v_" << name;
    }
}

void Generator::visit(ThisExpr* node) {
    out << "_thisContext";
}

void Generator::visit(BinaryExpr* node) {
    if (node->op.type == TokenType::StrictEq) {
        out << "JSValue(";
        node->left->accept(*this);
        out << ".strictEquals(";
        node->right->accept(*this);
        out << "))";
        return;
    }
    
    if (node->op.type == TokenType::In) {
        out << "JSValue(std::get<std::shared_ptr<JSObject>>(";
        node->right->accept(*this);
        out << ".value)->hasProperty(";
        node->left->accept(*this);
        out << ".toString()))";
        return;
    }

    if (node->op.type == TokenType::StrictNotEq) {
        out << "JSValue(!(";
        node->left->accept(*this);
        out << ".strictEquals(";
        node->right->accept(*this);
        out << ")))";
        return;
    }
    
    // Short-Circuit Logical Operators
    if (node->op.type == TokenType::LogicalAnd) {
        out << "([&]() -> JSValue { JSValue _tmp = ";
        node->left->accept(*this);
        out << "; if (_tmp.toBoolean()) return (";
        node->right->accept(*this);
        out << "); return _tmp; })()";
        return;
    }
    if (node->op.type == TokenType::LogicalOr) {
        out << "([&]() -> JSValue { JSValue _tmp = ";
        node->left->accept(*this);
        out << "; if (_tmp.toBoolean()) return _tmp; return (";
        node->right->accept(*this);
        out << "); })()";
        return;
    }
    
    if (node->op.type == TokenType::Exponent) {
        out << "JSValue(std::pow(";
        node->left->accept(*this);
        out << ".toNumber(), ";
        node->right->accept(*this);
        out << ".toNumber()))";
        return;
    }
    
    if (node->op.type == TokenType::UnsignedShiftRight) {
        out << "JSValue(static_cast<double>(static_cast<uint32_t>(";
        node->left->accept(*this);
        out << ".toNumber()) >> (static_cast<uint32_t>(";
        node->right->accept(*this);
        out << ".toNumber()) & 0x1F)))";
        return;
    }

    out << "JSValue((";
    node->left->accept(*this);
    out << " " << node->op.lexeme << " ";
    node->right->accept(*this);
    out << "))";
}

void Generator::visit(UnaryExpr* node) {
    if (node->op.type == TokenType::Typeof) {
        out << "JSValue(";
        node->right->accept(*this);
        out << ".typeOf())";
        return;
    }
    if (node->op.type == TokenType::BitNot) {
        out << "JSValue(~(";
        node->right->accept(*this);
        out << "))";
        return;
    }
    
    if (node->op.type == TokenType::LogicalNot) out << "JSValue(!(";
    else out << "JSValue(-(";
    
    node->right->accept(*this);
    
    if (node->op.type == TokenType::LogicalNot) out << ".toBoolean()))";
    else out << ".toNumber()))";
}

void Generator::visit(UpdateExpr* node) {
    if (node->isPrefix) {
        out << (node->op.type == TokenType::PlusPlus ? "++(" : "--(");
        node->operand->accept(*this);
        out << ")";
    } else {
        out << "(";
        node->operand->accept(*this);
        out << (node->op.type == TokenType::PlusPlus ? ")++" : ")--");
    }
}

void Generator::visit(AssignmentExpr* node) {
    if (node->op.type == TokenType::ExponentAssign) {
        node->target->accept(*this);
        out << " = JSValue(std::pow(";
        node->target->accept(*this);
        out << ".toNumber(), ";
        node->value->accept(*this);
        out << ".toNumber()))";
        return;
    }
    
    if (auto member = dynamic_cast<MemberExpr*>(node->target.get())) {
        if (node->op.type == TokenType::Assign) {
            member->object->accept(*this);
            out << ".setProperty(";
            if (member->computed) {
                member->propertyExpr->accept(*this);
            } else {
                out << "JSValue(\"" << member->property.lexeme << "\")";
            }
            out << ", ";
            node->value->accept(*this);
            out << ")";
            return;
        }
    }
    
    node->target->accept(*this);
    out << " " << node->op.lexeme << " ";
    node->value->accept(*this);
}

void Generator::visit(CallExpr* node) {
    if (auto id = dynamic_cast<IdentifierExpr*>(node->callee.get())) {
        if (id->name.lexeme == "String") {
            out << "JSValue((";
            if (!node->arguments.empty()) node->arguments[0]->accept(*this);
            else out << "JSValue()";
            out << ").toString())";
            return;
        }
        if (id->name.lexeme == "Number") {
            out << "JSValue((";
            if (!node->arguments.empty()) node->arguments[0]->accept(*this);
            else out << "JSValue()";
            out << ").toNumber())";
            return;
        }
        if (id->name.lexeme == "Boolean") {
            out << "JSValue((";
            if (!node->arguments.empty()) node->arguments[0]->accept(*this);
            else out << "JSValue()";
            out << ").toBoolean())";
            return;
        }
    }

    // Pattern Matching for Hackathon specific API calls and dynamic method calls
    if (auto member = dynamic_cast<MemberExpr*>(node->callee.get())) {
        if (auto id = dynamic_cast<IdentifierExpr*>(member->object.get())) {
            if (id->name.lexeme == "console" && member->property.lexeme == "log") {
                out << "console::log(";
                generateArguments(node->arguments);
                out << ")";
                return;
            }
            if (id->name.lexeme == "Math") {
                if (member->property.lexeme == "max" || member->property.lexeme == "min") {
                    out << "JSMath::" << member->property.lexeme << "(";
                    generateArguments(node->arguments);
                    out << ")";
                } else if (member->property.lexeme == "pow") {
                    out << "JSMath::pow(";
                    if (node->arguments.size() > 0) node->arguments[0]->accept(*this); else out << "JSValue()";
                    out << ", ";
                    if (node->arguments.size() > 1) node->arguments[1]->accept(*this); else out << "JSValue()";
                    out << ")";
                } else if (member->property.lexeme == "random") {
                    out << "JSMath::random()";
                } else {
                    out << "JSMath::" << member->property.lexeme << "(";
                    if (!node->arguments.empty()) node->arguments[0]->accept(*this); else out << "JSValue()";
                    out << ")";
                }
                return;
            }
            if (id->name.lexeme == "Date" && member->property.lexeme == "now") {
                out << "JSDate::now()";
                return;
            }
            if (id->name.lexeme == "Object" && member->property.lexeme == "keys") {
                out << "JSValue(JSObject::objectKeys(std::get<std::shared_ptr<JSObject>>(";
                if (!node->arguments.empty()) node->arguments[0]->accept(*this);
                out << ".value)))";
                return;
            }
            if (id->name.lexeme == "Object" && member->property.lexeme == "assign") {
                out << "([&]() -> JSValue { JSValue _target = ";
                if (node->arguments.size() > 0) node->arguments[0]->accept(*this);
                else out << "JSValue()";
                out << "; JSValue _source = ";
                if (node->arguments.size() > 1) node->arguments[1]->accept(*this);
                else out << "JSValue()";
                out << "; JSObject::assign(std::get<std::shared_ptr<JSObject>>(_target.value), std::get<std::shared_ptr<JSObject>>(_source.value)); return _target; })()";
                return;
            }
        }
        
        // Generic Dynamic Method Call (e.g. arr.push(1))
        if (!member->computed) {
            member->object->accept(*this);
            out << ".callMethod(\"" << member->property.lexeme << "\", ";
            generateArguments(node->arguments);
            out << ")";
            return;
        }
    }

    // Generic Call execution (Assumes the callee evaluates to a JSFunction)
    out << "std::get<std::shared_ptr<JSFunction>>(";
    node->callee->accept(*this);
    out << ".value)->call(";
    generateArguments(node->arguments);
    out << ")";
}

void Generator::visit(NewExpr* node) {
    if (auto id = dynamic_cast<IdentifierExpr*>(node->callee.get())) {
        if (id->name.lexeme == "Date") {
            out << "JSDate::create(";
            generateArguments(node->arguments);
            out << ")";
            return;
        }
    }
    // Generic fallback
    out << "std::get<std::shared_ptr<JSFunction>>(";
    node->callee->accept(*this);
    out << ".value)->call(";
    generateArguments(node->arguments);
    out << ")";
}

void Generator::visit(MemberExpr* node) {
    if (!node->computed) {
        if (auto id = dynamic_cast<IdentifierExpr*>(node->object.get())) {
            if (id->name.lexeme == "Math") {
                out << "JSMath::" << node->property.lexeme;
                return;
            }
        }
    }

    if (node->computed) {
        node->object->accept(*this);
        out << ".getProperty(JSValue(";
        node->propertyExpr->accept(*this);
        out << "))";
    } else {
        node->object->accept(*this);
        out << ".getProperty(JSValue(\"" << node->property.lexeme << "\"))";
    }
}

void Generator::visit(ArrayLiteralExpr* node) {
    out << "JSValue(JSArray::create(";
    generateArguments(node->elements);
    out << "))";
}

void Generator::visit(SpreadExpr* node) {
    out << "JSValue() /* Error: Unhandled SpreadExpr */";
}

void Generator::visit(ObjectLiteralExpr* node) {
    out << "([&]() -> JSValue {\n";
    indentLevel++;
    writeIndent();
    out << "auto obj = JSObject::create();\n";
    
    int spreadCounter = 0;
    for (const auto& prop : node->properties) {
        if (prop.isSpread) {
            writeIndent();
            out << "{ auto _spread" << spreadCounter << " = ";
            if (auto spread = dynamic_cast<SpreadExpr*>(prop.value.get())) {
                spread->expression->accept(*this);
            } else {
                prop.value->accept(*this);
            }
            out << ";\n";
            writeIndent();
            out << "if (_spread" << spreadCounter << ".isObject()) {\n";
            indentLevel++;
            writeIndent();
            out << "auto _s = std::get<std::shared_ptr<JSObject>>(_spread" << spreadCounter << ".value);\n";
            writeIndent();
            out << "for (auto& _p : _s->properties) obj->properties[_p.first] = _p.second;\n";
            indentLevel--;
            writeIndent();
            out << "} }\n";
            spreadCounter++;
        } else if (prop.isGetter) {
            writeIndent();
            out << "obj->defineGetter(";
            if (prop.isComputed) { prop.computedKey->accept(*this); }
            else { out << "JSValue(\"" << prop.key << "\")"; }
            out << ".toString(), std::get<std::shared_ptr<JSFunction>>(";
            prop.value->accept(*this);
            out << ".value));\n";
        } else if (prop.isSetter) {
            writeIndent();
            out << "obj->defineSetter(";
            if (prop.isComputed) { prop.computedKey->accept(*this); }
            else { out << "JSValue(\"" << prop.key << "\")"; }
            out << ".toString(), std::get<std::shared_ptr<JSFunction>>(";
            prop.value->accept(*this);
            out << ".value));\n";
        } else {
            writeIndent();
            out << "obj->properties[";
            if (prop.isComputed) { prop.computedKey->accept(*this); out << ".toString()"; }
            else { out << "\"" << prop.key << "\""; }
            out << "] = ";
            prop.value->accept(*this);
            out << ";\n";
        }
    }
    writeIndent();
    out << "return JSValue(obj);\n";
    indentLevel--;
    writeIndent();
    out << "})()";
}

void Generator::visit(FunctionExpr* node) {
    if (node->isArrow) {
        out << "JSValue(JSFunction::create([&](const std::vector<JSValue>& args, JSValue /* _unusedThis */) -> JSValue {\n";
    } else {
        out << "JSValue(JSFunction::create([&](const std::vector<JSValue>& args, JSValue _thisContext) -> JSValue {\n";
    }
    indentLevel++;
    
    for (size_t i = 0; i < node->params.size(); ++i) {
        writeIndent();
        out << "JSValue _v_" << node->params[i].lexeme << " = args.size() > " << i 
            << " ? args[" << i << "] : JSValue();\n";
    }
    
    if (node->hasRest) {
        writeIndent();
        out << "auto _restArr = JSArray::create({});\n";
        writeIndent();
        out << "for (size_t i = " << node->params.size() << "; i < args.size(); ++i) _restArr->push({args[i]});\n";
        writeIndent();
        out << "JSValue _v_" << node->restParam.lexeme << " = JSValue(_restArr);\n";
    }
    
    for (const auto& stmt : node->body->statements) {
        stmt->accept(*this);
    }
    writeIndent();
    out << "return JSValue();\n"; 
    
    indentLevel--;
    writeIndent();
    out << "}))";
}