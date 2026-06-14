#include "js_object.h"
#include "js_array.h" // CRITICAL: Required for objectKeys() to return an array

// ==========================================
// 1. INSTANCE METHODS
// ==========================================

JSValue& JSObject::operator[](const std::string& key) {
    // If the key doesn't exist, std::unordered_map automatically creates it.
    // Because JSValue's default constructor is JSUndefined, this perfectly
    // mimics JavaScript's behavior of returning 'undefined' for missing keys.
    return properties[key];
}

bool JSObject::hasProperty(const std::string& key) const {
    return properties.find(key) != properties.end();
}

std::vector<std::string> JSObject::keys() const {
    std::vector<std::string> keyList;
    for (const auto& pair : properties) {
        keyList.push_back(pair.first);
    }
    return keyList;
}

void JSObject::defineGetter(const std::string& key, std::shared_ptr<JSFunction> getter) {
    getters[key] = getter;
}

void JSObject::defineSetter(const std::string& key, std::shared_ptr<JSFunction> setter) {
    setters[key] = setter;
}

// ==========================================
// 2. STATIC FACTORY METHODS
// ==========================================

std::shared_ptr<JSObject> JSObject::create() {
    return std::make_shared<JSObject>();
}

// ==========================================
// 3. JAVASCRIPT OBJECT PROTOTYPE SIMULATORS
// ==========================================

// Simulates Object.assign() and Object Spread ({...obj})
void JSObject::assign(std::shared_ptr<JSObject> target, std::shared_ptr<JSObject> source) {
    if (!target || !source) return;
    
    for (const auto& pair : source->properties) {
        target->properties[pair.first] = pair.second; // Copies the value/reference over
    }
}

// Simulates Object.keys(obj)
std::shared_ptr<JSArray> JSObject::objectKeys(std::shared_ptr<JSObject> obj) {
    auto arr = JSArray::create();
    
    if (!obj) return arr; // Return empty array if the object is null
    
    for (const auto& pair : obj->properties) {
        // Push each string key into our JSArray instance
        arr->push({JSValue(pair.first)});
    }
    
    return arr;
}