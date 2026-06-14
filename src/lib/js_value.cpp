#include "js_value.h"
#include "js_array.h" // CRITICAL: Required for split() to return an array
#include "js_object.h" // Required for object property access
#include "js_function.h" // Required for map, filter, etc.

// ==========================================
// 1. TYPE COERCION (The tricky part of JS)
// ==========================================

std::string JSValue::toString() const {
    if (isUndefined()) return "undefined";
    if (isNull()) return "null";
    if (getType() == Type::Boolean) return std::get<bool>(value) ? "true" : "false";
    if (isString()) return *std::get<std::shared_ptr<std::string>>(value);
    if (isNumber()) {
        double num = std::get<double>(value);
        if (num == std::floor(num)) return std::to_string(static_cast<long long>(num)); // No decimals for integers
        return std::to_string(num);
    }
    if (isArray()) {
        auto arr = std::get<std::shared_ptr<JSArray>>(value);
        return "[" + arr->join(", ") + "]";
    }
    return "[object Object]"; // Fallback for reference types
}

double JSValue::toNumber() const {
    if (isUndefined()) return NAN;
    if (isNull()) return 0.0; // JS Quirk: Number(null) is 0
    if (getType() == Type::Boolean) return std::get<bool>(value) ? 1.0 : 0.0;
    if (isNumber()) return std::get<double>(value);
    if (isString()) {
        try { return std::stod(*std::get<std::shared_ptr<std::string>>(value)); } 
        catch (...) { return NAN; }
    }
    return NAN;
}

bool JSValue::toBoolean() const {
    if (isUndefined() || isNull()) return false;
    if (getType() == Type::Boolean) return std::get<bool>(value);
    if (isNumber()) {
        double num = std::get<double>(value);
        return (num != 0.0 && !std::isnan(num)); // 0 and NaN are falsy
    }
    if (isString()) return *std::get<std::shared_ptr<std::string>>(value) != ""; // Empty string is falsy
    return true; // Objects and Arrays are always truthy
}

// ==========================================
// 2. ARITHMETIC OPERATORS
// ==========================================

JSValue JSValue::operator+(const JSValue& other) const {
    if (isString() || other.isString()) {
        return JSValue(this->toString() + other.toString()); // String concatenation
    }
    return JSValue(this->toNumber() + other.toNumber()); // Math addition
}

JSValue JSValue::operator-(const JSValue& other) const { return JSValue(this->toNumber() - other.toNumber()); }
JSValue JSValue::operator*(const JSValue& other) const { return JSValue(this->toNumber() * other.toNumber()); }
JSValue JSValue::operator/(const JSValue& other) const { return JSValue(this->toNumber() / other.toNumber()); }
JSValue JSValue::operator%(const JSValue& other) const { return JSValue(std::fmod(this->toNumber(), other.toNumber())); }

// ==========================================
// 3. ASSIGNMENT & LOGICAL OPERATORS
// ==========================================

JSValue& JSValue::operator+=(const JSValue& other) { *this = *this + other; return *this; }
JSValue& JSValue::operator-=(const JSValue& other) { *this = *this - other; return *this; }
JSValue& JSValue::operator*=(const JSValue& other) { *this = *this * other; return *this; }
JSValue& JSValue::operator/=(const JSValue& other) { *this = *this / other; return *this; }
JSValue& JSValue::operator%=(const JSValue& other) { *this = *this % other; return *this; }

// ==========================================
// 3.5 BITWISE OPERATORS
// ==========================================

JSValue JSValue::operator&(const JSValue& other) const {
    return JSValue(static_cast<double>(static_cast<int32_t>(this->toNumber()) & static_cast<int32_t>(other.toNumber())));
}
JSValue JSValue::operator|(const JSValue& other) const {
    return JSValue(static_cast<double>(static_cast<int32_t>(this->toNumber()) | static_cast<int32_t>(other.toNumber())));
}
JSValue JSValue::operator^(const JSValue& other) const {
    return JSValue(static_cast<double>(static_cast<int32_t>(this->toNumber()) ^ static_cast<int32_t>(other.toNumber())));
}
JSValue JSValue::operator<<(const JSValue& other) const {
    return JSValue(static_cast<double>(static_cast<int32_t>(this->toNumber()) << (static_cast<uint32_t>(other.toNumber()) & 0x1F)));
}
JSValue JSValue::operator>>(const JSValue& other) const {
    return JSValue(static_cast<double>(static_cast<int32_t>(this->toNumber()) >> (static_cast<uint32_t>(other.toNumber()) & 0x1F)));
}
JSValue JSValue::operator~() const {
    return JSValue(static_cast<double>(~static_cast<int32_t>(this->toNumber())));
}

JSValue JSValue::operator++(int) {
    JSValue old = *this;
    *this = *this + JSValue(1);
    return old;
}

JSValue JSValue::operator--(int) {
    JSValue old = *this;
    *this = *this - JSValue(1);
    return old;
}

JSValue& JSValue::operator++() {
    *this = *this + JSValue(1);
    return *this;
}

JSValue& JSValue::operator--() {
    *this = *this - JSValue(1);
    return *this;
}

// Logical NOT (Handles JS `!value`)
bool JSValue::operator!() const {
    return !this->toBoolean();
}

// ==========================================
// 4. EQUALITY & RELATIONAL OPERATORS
// ==========================================

// STRICT EQUALITY (JS ===)
bool JSValue::strictEquals(const JSValue& other) const {
    if (value.index() != other.value.index()) return false;
    
    // If both are numbers, check for NaN since NaN !== NaN
    if (std::holds_alternative<double>(value) && std::holds_alternative<double>(other.value)) {
        double v1 = std::get<double>(value);
        double v2 = std::get<double>(other.value);
        if (std::isnan(v1) || std::isnan(v2)) return false;
        return v1 == v2;
    }
    if (isString() && other.isString()) {
        return *std::get<std::shared_ptr<std::string>>(value) == *std::get<std::shared_ptr<std::string>>(other.value);
    }
    
    return value == other.value;
}

std::string JSValue::typeOf() const {
    if (std::holds_alternative<JSUndefined>(value)) return "undefined";
    if (std::holds_alternative<JSNull>(value)) return "object";
    if (std::holds_alternative<bool>(value)) return "boolean";
    if (std::holds_alternative<double>(value)) return "number";
    if (std::holds_alternative<std::shared_ptr<std::string>>(value)) return "string";
    if (std::holds_alternative<std::shared_ptr<JSFunction>>(value)) return "function";
    return "object";
}

// LOOSE EQUALITY (JS ==)
bool JSValue::operator==(const JSValue& other) const {
    if (getType() == other.getType()) return strictEquals(other);

    // JS Quirks: null and undefined are loosely equal to each other, but nothing else
    if ((isUndefined() || isNull()) && (other.isUndefined() || other.isNull())) return true;

    if (isNumber() && other.isString()) return this->toNumber() == other.toNumber();
    if (isString() && other.isNumber()) return this->toNumber() == other.toNumber();

    if (getType() == Type::Boolean) return JSValue(this->toNumber()) == other;
    if (other.getType() == Type::Boolean) return *this == JSValue(other.toNumber());

    return false;
}

bool JSValue::operator!=(const JSValue& other) const {
    return !(*this == other);
}

bool JSValue::operator<(const JSValue& other) const {
    if (isString() && other.isString()) return *std::get<std::shared_ptr<std::string>>(value) < *std::get<std::shared_ptr<std::string>>(other.value);
    return this->toNumber() < other.toNumber();
}

bool JSValue::operator>(const JSValue& other) const {
    if (isString() && other.isString()) return *std::get<std::shared_ptr<std::string>>(value) > *std::get<std::shared_ptr<std::string>>(other.value);
    return this->toNumber() > other.toNumber();
}

bool JSValue::operator<=(const JSValue& other) const {
    if (isString() && other.isString()) return *std::get<std::shared_ptr<std::string>>(value) <= *std::get<std::shared_ptr<std::string>>(other.value);
    return this->toNumber() <= other.toNumber();
}

bool JSValue::operator>=(const JSValue& other) const {
    if (isString() && other.isString()) return *std::get<std::shared_ptr<std::string>>(value) >= *std::get<std::shared_ptr<std::string>>(other.value);
    return this->toNumber() >= other.toNumber();
}

// ==========================================
// 5. STRING METHODS
// ==========================================

JSValue JSValue::toUpperCase() const {
    if (!isString()) return JSValue("");
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return JSValue(str);
}

JSValue JSValue::toLowerCase() const {
    if (!isString()) return JSValue("");
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return JSValue(str);
}

JSValue JSValue::indexOf(const std::string& search) const {
    if (!isString()) return JSValue(-1);
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    size_t pos = str.find(search);
    if (pos == std::string::npos) return JSValue(-1);
    return JSValue(static_cast<double>(pos));
}

JSValue JSValue::includes(const std::string& search) const {
    return JSValue(this->indexOf(search).toNumber() != -1);
}

JSValue JSValue::slice(int start, int end) const {
    if (!isString()) return JSValue("");
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    int len = str.length();
    
    if (start < 0) start = std::max(len + start, 0);
    if (end < 0) end = (end == -1) ? len : std::max(len + end, 0);
    if (end > len) end = len;
    
    if (start >= end) return JSValue("");
    return JSValue(str.substr(start, end - start));
}

JSValue JSValue::substring(int start, int end) const {
    if (!isString()) return JSValue("");
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    if (start < 0) start = 0;
    if (end < 0) end = 0;
    if (start > end) std::swap(start, end); // substring swaps if start > end
    return JSValue(str.substr(start, end - start));
}

JSValue JSValue::trim() const {
    if (!isString()) return JSValue("");
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    size_t first = str.find_first_not_of(" \n\r\t");
    size_t last = str.find_last_not_of(" \n\r\t");
    if (first == std::string::npos) return JSValue(""); 
    return JSValue(str.substr(first, (last - first + 1)));
}

JSValue JSValue::startsWith(const std::string& search) const {
    if (!isString()) return JSValue(false);
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    return JSValue(str.find(search) == 0);
}

JSValue JSValue::endsWith(const std::string& search) const {
    if (!isString()) return JSValue(false);
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    if (str.length() < search.length()) return JSValue(false);
    return JSValue(str.compare(str.length() - search.length(), search.length(), search) == 0);
}

JSValue JSValue::replace(const std::string& search, const std::string& replacement) const {
    if (!isString()) return *this;
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    size_t pos = str.find(search);
    if (pos != std::string::npos) {
        str.replace(pos, search.length(), replacement);
    }
    return JSValue(str);
}

JSValue JSValue::replaceAll(const std::string& search, const std::string& replacement) const {
    if (!isString() || search.empty()) return *this;
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    size_t pos = 0;
    while ((pos = str.find(search, pos)) != std::string::npos) {
        str.replace(pos, search.length(), replacement);
        pos += replacement.length();
    }
    return JSValue(str);
}

// CRITICAL: Connects strings to arrays for Hackathon Test Case 5
JSValue JSValue::split(const std::string& delimiter) const {
    if (!isString()) return JSArray::create();
    std::string str = *std::get<std::shared_ptr<std::string>>(value);
    auto arr = JSArray::create();
    
    // JS Quirk: "".split("") returns []
    if (str.empty() && delimiter.empty()) return JSValue(arr);

    if (delimiter.empty()) {
        for (char c : str) arr->push({JSValue(std::string(1, c))});
        return JSValue(arr);
    }

    size_t pos_start = 0, pos_end;
    while ((pos_end = str.find(delimiter, pos_start)) != std::string::npos) {
        arr->push({JSValue(str.substr(pos_start, pos_end - pos_start))});
        pos_start = pos_end + delimiter.length();
    }
    arr->push({JSValue(str.substr(pos_start))});
    return JSValue(arr);
}

// ==========================================
// 6. OBJECT AND ARRAY DISPATCH
// ==========================================

JSValue& JSValue::operator[](const JSValue& key) {
    if (isArray()) {
        auto arr = std::get<std::shared_ptr<JSArray>>(value);
        if (key.isString() && key.toString() == "length") {
            static JSValue len;
            len = JSValue(arr->length());
            return len;
        }
        return arr->operator[](static_cast<size_t>(key.toNumber()));
    }
    if (isObject()) {
        auto obj = std::get<std::shared_ptr<JSObject>>(value);
        return obj->operator[](key.toString());
    }
    
    // Primitives shouldn't be assigned properties, but we must return a reference.
    static JSValue dummy;
    dummy = JSValue(); // reset
    return dummy;
}

JSValue JSValue::getProperty(const JSValue& key) const {
    if (isObject()) {
        auto obj = std::get<std::shared_ptr<JSObject>>(value);
        std::string k = key.toString();
        if (obj->getters.find(k) != obj->getters.end()) {
            return obj->getters[k]->call({}, *this);
        }
        if (obj->hasProperty(k)) {
            return obj->properties[k];
        }
        return JSValue();
    }
    if (isArray()) {
        auto arr = std::get<std::shared_ptr<JSArray>>(value);
        if (key.isString() && key.toString() == "length") {
            return JSValue(static_cast<double>(arr->length()));
        }
        return arr->operator[](static_cast<size_t>(key.toNumber()));
    }
    if (isString()) {
        if (key.isString() && key.toString() == "length") {
            return JSValue(static_cast<double>(std::get<std::shared_ptr<std::string>>(value)->length()));
        }
    }
    return JSValue();
}

void JSValue::setProperty(const JSValue& key, const JSValue& val) {
    if (isObject()) {
        auto obj = std::get<std::shared_ptr<JSObject>>(value);
        std::string k = key.toString();
        if (obj->setters.find(k) != obj->setters.end()) {
            obj->setters[k]->call({val}, *this);
            return;
        }
        obj->properties[k] = val;
        return;
    }
    if (isArray()) {
        auto arr = std::get<std::shared_ptr<JSArray>>(value);
        arr->operator[](static_cast<size_t>(key.toNumber())) = val;
        return;
    }
}

JSValue JSValue::callMethod(const std::string& name, const std::vector<JSValue>& args) const {
    if (isArray()) {
        auto arr = std::get<std::shared_ptr<JSArray>>(value);
        if (name == "push") return arr->push(args);
        if (name == "pop") return arr->pop();
        if (name == "shift") return arr->shift();
        if (name == "unshift") return arr->unshift(args);
        if (name == "slice") {
            int start = args.size() > 0 ? args[0].toNumber() : 0;
            int end = args.size() > 1 ? args[1].toNumber() : -1;
            return JSValue(arr->slice(start, end));
        }
        if (name == "splice") {
            int start = args.size() > 0 ? args[0].toNumber() : 0;
            int deleteCount = args.size() > 1 ? args[1].toNumber() : 0;
            return JSValue(arr->splice(start, deleteCount));
        }
        if (name == "concat") return JSValue(arr->concat(args));
        if (name == "includes") return arr->includes(args.size() > 0 ? args[0] : JSValue());
        if (name == "indexOf") return arr->indexOf(args.size() > 0 ? args[0] : JSValue());
        if (name == "join") return JSValue(arr->join(args.size() > 0 ? args[0].toString() : ","));
        if (name == "reverse") { arr->reverse(); return *this; }
        if (name == "sort") {
            arr->sort(args.size() > 0 && args[0].getType() == Type::Function ? std::get<std::shared_ptr<JSFunction>>(args[0].value) : nullptr);
            return *this;
        }
        
        // Higher order functions
        if (name == "map") return JSValue(arr->map(args.size() > 0 ? args[0] : JSValue()));
        if (name == "filter") return JSValue(arr->filter(args.size() > 0 ? args[0] : JSValue()));
        if (name == "reduce") return arr->reduce(args.size() > 0 ? args[0] : JSValue(), args.size() > 1 ? args[1] : JSValue());
        if (name == "find") return arr->find(args.size() > 0 ? args[0] : JSValue());
        if (name == "some") return arr->some(args.size() > 0 ? args[0] : JSValue());
        if (name == "every") return arr->every(args.size() > 0 ? args[0] : JSValue());
    }
    
    if (isString()) {
        if (name == "toUpperCase") return toUpperCase();
        if (name == "toLowerCase") return toLowerCase();
        if (name == "trim") return trim();
        if (name == "slice") {
            int start = args.size() > 0 ? args[0].toNumber() : 0;
            int end = args.size() > 1 ? args[1].toNumber() : -1;
            return slice(start, end);
        }
        if (name == "substring") {
            int start = args.size() > 0 ? args[0].toNumber() : 0;
            int end = args.size() > 1 ? args[1].toNumber() : -1;
            return substring(start, end);
        }
        if (name == "replace") {
            if (args.size() >= 2) return replace(args[0].toString(), args[1].toString());
            return *this;
        }
        if (name == "replaceAll") {
            if (args.size() >= 2) return replaceAll(args[0].toString(), args[1].toString());
            return *this;
        }
        if (name == "includes") return includes(args.size() > 0 ? args[0].toString() : "");
        if (name == "startsWith") return startsWith(args.size() > 0 ? args[0].toString() : "");
        if (name == "endsWith") return endsWith(args.size() > 0 ? args[0].toString() : "");
        if (name == "indexOf") return indexOf(args.size() > 0 ? args[0].toString() : "");
        if (name == "split") return split(args.size() > 0 ? args[0].toString() : "");
    }
    
    if (isObject()) {
        auto obj = std::get<std::shared_ptr<JSObject>>(value);
        if (obj->hasProperty(name)) {
            JSValue prop = obj->properties[name];
            if (prop.getType() == Type::Function) {
                auto func = std::get<std::shared_ptr<JSFunction>>(prop.value);
                return func->call(args, *this);
            }
        }
    }

    return JSValue();
}

// ==========================================
// 7. CONSOLE POLYFILL
// ==========================================

std::ostream& operator<<(std::ostream& os, const JSValue& val) {
    os << val.toString();
    return os;
}

namespace console {
    JSValue log(const std::vector<JSValue>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            std::cout << args[i].toString();
            if (i < args.size() - 1) std::cout << " ";
        }
        std::cout << std::endl;
        return JSValue();
    }
}
