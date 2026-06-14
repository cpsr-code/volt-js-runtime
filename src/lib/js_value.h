#pragma once
#include <iostream>
#include <string>
#include <variant>
#include <memory>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>

// Forward declarations for reference types
class JSObject;
class JSArray;
class JSFunction;

// Distinct structs to separate Null and Undefined primitives
struct JSUndefined {
    bool operator==(const JSUndefined&) const { return true; }
};
struct JSNull {
    bool operator==(const JSNull&) const { return true; }
};

class JSValue {
public:
    // The master variant holding all 7 JS data types
    std::variant<
        JSUndefined,                      // Undefined
        JSNull,                           // Null
        bool,                             // Boolean
        double,                           // Number
        std::shared_ptr<std::string>,     // String
        std::shared_ptr<JSObject>,        // Object
        std::shared_ptr<JSArray>,         // Array
        std::shared_ptr<JSFunction>       // Function
    > value;

    enum class Type { Undefined, Null, Boolean, Number, String, Object, Array, Function };

    // --- Constructors ---
    JSValue() : value(JSUndefined{}) {}                  // Default is undefined
    JSValue(JSUndefined) : value(JSUndefined{}) {}
    JSValue(JSNull) : value(JSNull{}) {}
    JSValue(bool b) : value(b) {}
    JSValue(double d) : value(d) {}
    JSValue(int i) : value(static_cast<double>(i)) {}
    JSValue(const std::string& s) : value(std::make_shared<std::string>(s)) {}
    JSValue(const char* s) : value(std::make_shared<std::string>(s)) {}

    // Reference Type Constructors
    JSValue(std::shared_ptr<JSObject> obj) : value(obj) {}
    JSValue(std::shared_ptr<JSArray> arr) : value(arr) {}
    JSValue(std::shared_ptr<JSFunction> func) : value(func) {}

    // --- Type Checking ---
    Type getType() const { return static_cast<Type>(value.index()); }
    bool isUndefined() const { return getType() == Type::Undefined; }
    bool isNull() const { return getType() == Type::Null; }
    bool isString() const { return getType() == Type::String; }
    bool isNumber() const { return getType() == Type::Number; }
    bool isArray() const { return getType() == Type::Array; }
    bool isObject() const { return getType() == Type::Object; }
    
    // --- Type Conversions ---
    std::string toString() const;
    double toNumber() const;
    bool toBoolean() const;
    std::string typeOf() const;

    // --- Arithmetic Operators ---
    JSValue operator+(const JSValue& other) const;
    JSValue operator-(const JSValue& other) const;
    JSValue operator*(const JSValue& other) const;
    JSValue operator/(const JSValue& other) const;
    JSValue operator%(const JSValue& other) const; 
    
    // --- Assignment Operators ---
    JSValue& operator+=(const JSValue& other);
    JSValue& operator-=(const JSValue& other);
    JSValue& operator*=(const JSValue& other);
    JSValue& operator/=(const JSValue& other);
    JSValue& operator%=(const JSValue& other);

    // --- Bitwise Operators ---
    JSValue operator&(const JSValue& other) const;
    JSValue operator|(const JSValue& other) const;
    JSValue operator^(const JSValue& other) const;
    JSValue operator<<(const JSValue& other) const;
    JSValue operator>>(const JSValue& other) const;
    JSValue operator~() const;

    JSValue operator++(int); // Postfix
    JSValue operator--(int); // Postfix
    JSValue& operator++();   // Prefix
    JSValue& operator--();   // Prefix

    // --- Logical Operators ---
    bool operator!() const; // Handles JS `!x`
    
    // --- Equality Operators ---
    bool operator==(const JSValue& other) const;   // Handles JS `==` (Loose Coercion)
    bool operator!=(const JSValue& other) const;   // Handles JS `!=`
    bool strictEquals(const JSValue& other) const; // Handles JS `===` (Strict)

    // --- Relational Operators ---
    bool operator<(const JSValue& other) const;
    bool operator>(const JSValue& other) const;
    bool operator<=(const JSValue& other) const;
    bool operator>=(const JSValue& other) const;

    // --- JavaScript String Methods ---
    JSValue toUpperCase() const;
    JSValue toLowerCase() const;
    JSValue trim() const;
    JSValue slice(int start, int end = -1) const;
    JSValue substring(int start, int end) const;
    JSValue replace(const std::string& search, const std::string& replacement) const;
    JSValue replaceAll(const std::string& search, const std::string& replacement) const;
    JSValue includes(const std::string& search) const;
    JSValue startsWith(const std::string& search) const;
    JSValue endsWith(const std::string& search) const;
    JSValue indexOf(const std::string& search) const;
    
    // Split connects Strings to Arrays (e.g., "a,b".split(","))
    JSValue split(const std::string& delimiter) const; 

    // --- Object & Array Methods ---
    JSValue& operator[](const JSValue& key);
    JSValue getProperty(const JSValue& key) const;
    void setProperty(const JSValue& key, const JSValue& val);
    JSValue callMethod(const std::string& name, const std::vector<JSValue>& args) const;

    // Helper for console.log
    friend std::ostream& operator<<(std::ostream& os, const JSValue& val);
};

// Global console object polyfill
namespace console {
    JSValue log(const std::vector<JSValue>& args);
}