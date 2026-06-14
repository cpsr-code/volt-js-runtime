#pragma once
#include "js_value.h"
#include <vector>
#include <string>
#include <memory>
#include <functional>

// Forward declaration to prevent circular dependencies
class JSFunction;

class JSArray : public std::enable_shared_from_this<JSArray> {
public:
    std::vector<JSValue> elements;

    // Constructors
    JSArray() = default;
    JSArray(std::vector<JSValue> init) : elements(std::move(init)) {}

    // Overload [] to mimic JS array indexing: arr[0] = 5
    JSValue& operator[](size_t index);
    
    // Property
    double length() const { return static_cast<double>(elements.size()); }

    // --- Mutators (Modify the original array) ---
    JSValue push(const std::vector<JSValue>& args);
    JSValue pop();
    JSValue shift();
    JSValue unshift(const std::vector<JSValue>& args);
    std::shared_ptr<JSArray> splice(int start, int deleteCount);
    void reverse();
    
    // JS sort optionally takes a comparison function
    void sort(std::shared_ptr<JSFunction> compareFn = nullptr);
    
    // --- Accessors (Return a new value/array) ---
    std::shared_ptr<JSArray> slice(int start, int end = -1) const;
    
    // Concat merges arrays/values. Crucial for the Spread operator (...arr)
    std::shared_ptr<JSArray> concat(const std::vector<JSValue>& items) const;
    
    JSValue includes(const JSValue& searchElement) const;
    JSValue indexOf(const JSValue& searchElement) const;
    std::string join(const std::string& separator = ",") const;

    // --- Iterators / Higher-Order Functions ---
    // These take JSValue callbacks that we will cast to JSFunction
    
    std::shared_ptr<JSArray> map(JSValue callback);
    std::shared_ptr<JSArray> filter(JSValue callback);
    JSValue reduce(JSValue callback, JSValue initialValue = JSValue());
    JSValue find(JSValue callback);
    JSValue some(JSValue callback);
    JSValue every(JSValue callback);

    // Factory method
    static std::shared_ptr<JSArray> create(std::vector<JSValue> init = {});
};