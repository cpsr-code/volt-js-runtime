#include "js_array.h"
#include "js_function.h" // Required to execute the callback in sort()
#include <algorithm>
#include <sstream>

JSValue& JSArray::operator[](size_t index) {
    // In JS, if you do arr[100] = 1 on an empty array, it fills the gaps with 'undefined'.
    if (index >= elements.size()) {
        elements.resize(index + 1, JSValue()); // Fill with default (Undefined)
    }
    return elements[index];
}

std::shared_ptr<JSArray> JSArray::create(std::vector<JSValue> init) {
    return std::make_shared<JSArray>(init);
}

// ==========================================
// MUTATORS
// ==========================================

JSValue JSArray::push(const std::vector<JSValue>& args) {
    for (const auto& arg : args) {
        elements.push_back(arg);
    }
    return JSValue(length());
}

JSValue JSArray::pop() {
    if (elements.empty()) return JSValue(); // Undefined
    JSValue last = elements.back();
    elements.pop_back();
    return last;
}

JSValue JSArray::shift() {
    if (elements.empty()) return JSValue();
    JSValue first = elements.front();
    elements.erase(elements.begin());
    return first;
}

JSValue JSArray::unshift(const std::vector<JSValue>& args) {
    elements.insert(elements.begin(), args.begin(), args.end());
    return JSValue(length());
}

void JSArray::reverse() {
    std::reverse(elements.begin(), elements.end());
}

std::shared_ptr<JSArray> JSArray::splice(int start, int deleteCount) {
    int len = elements.size();
    if (start < 0) start = std::max(len + start, 0);
    else start = std::min(start, len);

    deleteCount = std::min(std::max(deleteCount, 0), len - start);

    std::vector<JSValue> removedItems(elements.begin() + start, elements.begin() + start + deleteCount);
    elements.erase(elements.begin() + start, elements.begin() + start + deleteCount);

    return JSArray::create(removedItems);
}

// JS sort converts to strings by default, unless a compare function is provided
void JSArray::sort(std::shared_ptr<JSFunction> compareFn) {
    if (compareFn) {
        std::sort(elements.begin(), elements.end(), [&compareFn](const JSValue& a, const JSValue& b) {
            JSValue result = compareFn->call({a, b});
            return result.toNumber() < 0; // If negative, 'a' comes before 'b'
        });
    } else {
        std::sort(elements.begin(), elements.end(), [](const JSValue& a, const JSValue& b) {
            return a.toString() < b.toString(); 
        });
    }
}

// ==========================================
// ACCESSORS
// ==========================================

std::string JSArray::join(const std::string& separator) const {
    std::ostringstream oss;
    for (size_t i = 0; i < elements.size(); ++i) {
        // JS quirk: null or undefined in an array join become empty strings
        if (!elements[i].isUndefined() && !elements[i].isNull()) {
            oss << elements[i].toString();
        }
        if (i != elements.size() - 1) oss << separator;
    }
    return oss.str();
}

JSValue JSArray::indexOf(const JSValue& searchElement) const {
    for (size_t i = 0; i < elements.size(); ++i) {
        if (elements[i].strictEquals(searchElement)) return JSValue(static_cast<double>(i));
    }
    return JSValue(-1);
}

JSValue JSArray::includes(const JSValue& searchElement) const {
    return JSValue(indexOf(searchElement).toNumber() != -1);
}

std::shared_ptr<JSArray> JSArray::slice(int start, int end) const {
    int len = elements.size();
    if (start < 0) start = std::max(len + start, 0);
    if (end < 0) end = (end == -1) ? len : std::max(len + end, 0);
    if (end > len) end = len;

    if (start >= end) return JSArray::create();
    
    std::vector<JSValue> sliced(elements.begin() + start, elements.begin() + end);
    return JSArray::create(sliced);
}

// Concat merges arrays and values (Handles [...arr1, ...arr2])
std::shared_ptr<JSArray> JSArray::concat(const std::vector<JSValue>& items) const {
    auto result = JSArray::create(this->elements); // Copy current elements
    
    for (const auto& item : items) {
        if (item.isArray()) {
            // If it's an array, push its elements (Spread behavior)
            auto arr = std::get<std::shared_ptr<JSArray>>(item.value);
            result->elements.insert(result->elements.end(), arr->elements.begin(), arr->elements.end());
        } else {
            // Otherwise, just push the item
            result->elements.push_back(item);
        }
    }
    return result;
}

// ==========================================
// ITERATORS (Higher-Order Functions)
// ==========================================

std::shared_ptr<JSArray> JSArray::map(JSValue callback) {
    auto result = JSArray::create();
    if (callback.getType() != JSValue::Type::Function) return result;
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);
    
    std::vector<JSValue> args(3);
    args[2] = JSValue(shared_from_this());
    for (size_t i = 0; i < elements.size(); ++i) {
        args[0] = elements[i];
        args[1] = JSValue(static_cast<double>(i));
        result->elements.push_back(func->call(args));
    }
    return result;
}

std::shared_ptr<JSArray> JSArray::filter(JSValue callback) {
    auto result = JSArray::create();
    if (callback.getType() != JSValue::Type::Function) return result;
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);
    
    std::vector<JSValue> args(3);
    args[2] = JSValue(shared_from_this());
    for (size_t i = 0; i < elements.size(); ++i) {
        args[0] = elements[i];
        args[1] = JSValue(static_cast<double>(i));
        if (func->call(args).toBoolean()) {
            result->elements.push_back(elements[i]);
        }
    }
    return result;
}

JSValue JSArray::reduce(JSValue callback, JSValue initialValue) {
    size_t startIndex = 0;
    JSValue accumulator = initialValue;

    if (initialValue.isUndefined() && !elements.empty()) {
        accumulator = elements[0];
        startIndex = 1;
    }

    if (callback.getType() != JSValue::Type::Function) return accumulator;
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);

    std::vector<JSValue> args(4);
    args[3] = JSValue(shared_from_this());
    for (size_t i = startIndex; i < elements.size(); ++i) {
        args[0] = accumulator;
        args[1] = elements[i];
        args[2] = JSValue(static_cast<double>(i));
        accumulator = func->call(args);
    }
    return accumulator;
}

JSValue JSArray::find(JSValue callback) {
    if (callback.getType() != JSValue::Type::Function) return JSValue();
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);
    
    std::vector<JSValue> args(3);
    args[2] = JSValue(shared_from_this());
    for (size_t i = 0; i < elements.size(); ++i) {
        args[0] = elements[i];
        args[1] = JSValue(static_cast<double>(i));
        if (func->call(args).toBoolean()) return elements[i];
    }
    return JSValue(); // Undefined
}

JSValue JSArray::some(JSValue callback) {
    if (callback.getType() != JSValue::Type::Function) return JSValue(false);
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);
    
    std::vector<JSValue> args(3);
    args[2] = JSValue(shared_from_this());
    for (size_t i = 0; i < elements.size(); ++i) {
        args[0] = elements[i];
        args[1] = JSValue(static_cast<double>(i));
        if (func->call(args).toBoolean()) return JSValue(true);
    }
    return JSValue(false);
}

JSValue JSArray::every(JSValue callback) {
    if (callback.getType() != JSValue::Type::Function) return JSValue(true);
    auto func = std::get<std::shared_ptr<JSFunction>>(callback.value);
    
    std::vector<JSValue> args(3);
    args[2] = JSValue(shared_from_this());
    for (size_t i = 0; i < elements.size(); ++i) {
        args[0] = elements[i];
        args[1] = JSValue(static_cast<double>(i));
        if (!func->call(args).toBoolean()) return JSValue(false);
    }
    return JSValue(true);
}