#include "js_function.h"
#include "js_array.h" // Required to unpack arguments in the apply() method

// ==========================================
// CONSTRUCTOR & FACTORY
// ==========================================

JSFunction::JSFunction(std::function<JSValue(const std::vector<JSValue>&, JSValue)> func) 
    : callable(std::move(func)) {}

std::shared_ptr<JSFunction> JSFunction::create(std::function<JSValue(const std::vector<JSValue>&, JSValue)> func) {
    return std::make_shared<JSFunction>(std::move(func));
}

// ==========================================
// EXECUTION LOGIC
// ==========================================

JSValue JSFunction::call(const std::vector<JSValue>& args, JSValue thisContext) const {
    if (callable) {
        return callable(args, thisContext);
    }
    // In JS, calling an empty or undefined function throws a TypeError.
    // For our robust C++ transpiler, safely returning Undefined prevents hard C++ crashes.
    return JSValue(); 
}

// ==========================================
// PROTOTYPE METHODS
// ==========================================

// Simulates the JS `Function.prototype.apply()` method
JSValue JSFunction::apply(std::shared_ptr<JSObject> thisArg, std::shared_ptr<JSArray> argsArray) const {
    std::vector<JSValue> args;
    
    // Unpack the JSArray into a standard C++ vector to pass into our callable
    if (argsArray) {
        args = argsArray->elements;
    }
    
    // Note: In a full V8 engine, 'thisArg' would bind to the lexical environment.
    // We pass thisArg to the call to support dynamic `this` bindings.
    return call(args, JSValue(thisArg));
}