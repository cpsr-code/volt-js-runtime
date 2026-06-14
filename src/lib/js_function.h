#pragma once
#include "js_value.h"
#include <vector>
#include <memory>
#include <functional>

// Forward declaration to prevent circular dependencies
class JSArray;
class JSObject;

class JSFunction : public std::enable_shared_from_this<JSFunction> {
public:
    // The actual executable C++ code block.
    // It always takes a vector of JSValues to safely simulate JS argument spreading,
    // and a JSValue for the `this` context.
    std::function<JSValue(const std::vector<JSValue>& args, JSValue thisContext)> callable;

    // Constructor
    JSFunction(std::function<JSValue(const std::vector<JSValue>&, JSValue)> func);

    // Core execution method
    JSValue call(const std::vector<JSValue>& args = {}, JSValue thisContext = JSValue()) const;

    // Simulates the JS `Function.prototype.apply()` method
    JSValue apply(std::shared_ptr<JSObject> thisArg, std::shared_ptr<JSArray> argsArray) const;

    // Factory method to safely instantiate on the heap
    static std::shared_ptr<JSFunction> create(std::function<JSValue(const std::vector<JSValue>&, JSValue)> func);
};