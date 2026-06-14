#pragma once
#include "js_value.h"
#include <vector>
#include <cmath>

namespace JSMath {
    // Constants
    extern const JSValue PI;
    extern const JSValue E;

    // Methods
    JSValue abs(const JSValue& val);
    JSValue ceil(const JSValue& val);
    JSValue floor(const JSValue& val);
    JSValue round(const JSValue& val);
    JSValue pow(const JSValue& base, const JSValue& exp);
    JSValue sqrt(const JSValue& val);
    
    // JS Math.max/min take any number of arguments, so we pass them as a vector
    JSValue max(const std::vector<JSValue>& args);
    JSValue min(const std::vector<JSValue>& args);
    
    // Random number generation (0 inclusive to 1 exclusive)
    JSValue random();
}
