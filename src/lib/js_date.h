#pragma once
#include "js_value.h"
#include <memory>
#include <vector>

// Forward declarations
class JSObject;

namespace JSDate {
    
    // Simulates static method: Date.now()
    JSValue now(const std::vector<JSValue>& args = {});

    // Simulates the constructor: new Date()
    // We return it as a JSValue (containing a JSObject) so it fits directly into our engine.
    JSValue create(const std::vector<JSValue>& args = {});
    
}