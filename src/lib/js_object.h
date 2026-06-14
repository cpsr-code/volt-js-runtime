#pragma once
#include "js_value.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>

// Forward declaration for the Array class
class JSArray;

class JSObject : public std::enable_shared_from_this<JSObject> {
public:
    std::unordered_map<std::string, JSValue> properties;
    std::unordered_map<std::string, std::shared_ptr<JSFunction>> getters;
    std::unordered_map<std::string, std::shared_ptr<JSFunction>> setters;

    // Default constructor
    JSObject() = default;

    // Overload [] to mimic JS object property access: obj["name"] = "Alice"
    JSValue& operator[](const std::string& key);

    // Helpers
    bool hasProperty(const std::string& key) const;
    
    // Returns a raw C++ vector of keys
    std::vector<std::string> keys() const;

    // Getters / Setters
    void defineGetter(const std::string& key, std::shared_ptr<JSFunction> getter);
    void defineSetter(const std::string& key, std::shared_ptr<JSFunction> setter);

    // --- Factory and Static Methods ---

    // Factory method to create new objects safely wrapped in smart pointers
    static std::shared_ptr<JSObject> create();

    // Simulates Object.assign() and Object Spread ({...obj})
    static void assign(std::shared_ptr<JSObject> target, std::shared_ptr<JSObject> source);

    // Simulates Object.keys(obj), returns a JSArray
    static std::shared_ptr<JSArray> objectKeys(std::shared_ptr<JSObject> obj);
};