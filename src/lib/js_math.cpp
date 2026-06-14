#include "js_math.h"
#include <random>
#include <chrono>
#include <algorithm>

namespace JSMath {

    // Initialize Constants
    const JSValue PI = JSValue(3.141592653589793);
    const JSValue E  = JSValue(2.718281828459045);

    JSValue abs(const JSValue& val) {
        return JSValue(std::abs(val.toNumber()));
    }

    JSValue ceil(const JSValue& val) {
        return JSValue(std::ceil(val.toNumber()));
    }

    JSValue floor(const JSValue& val) {
        return JSValue(std::floor(val.toNumber()));
    }

    JSValue round(const JSValue& val) {
        // JS Math.round rounds half up (e.g., 2.5 goes to 3, -2.5 goes to -2)
        double num = val.toNumber();
        if (std::isnan(num)) return JSValue(NAN);
        return JSValue(std::floor(num + 0.5));
    }

    JSValue pow(const JSValue& base, const JSValue& exp) {
        return JSValue(std::pow(base.toNumber(), exp.toNumber()));
    }

    JSValue sqrt(const JSValue& val) {
        return JSValue(std::sqrt(val.toNumber()));
    }

    JSValue max(const std::vector<JSValue>& args) {
        if (args.empty()) return JSValue(-INFINITY);
        
        double max_val = -INFINITY;
        for (const auto& arg : args) {
            double current = arg.toNumber();
            if (std::isnan(current)) return JSValue(NAN); // If any arg is NaN, JS returns NaN
            if (current > max_val) max_val = current;
        }
        return JSValue(max_val);
    }

    JSValue min(const std::vector<JSValue>& args) {
        if (args.empty()) return JSValue(INFINITY);
        
        double min_val = INFINITY;
        for (const auto& arg : args) {
            double current = arg.toNumber();
            if (std::isnan(current)) return JSValue(NAN);
            if (current < min_val) min_val = current;
        }
        return JSValue(min_val);
    }

    JSValue random() {
        // Thread-local static PRNG ensures it only initializes once and is fast
        static thread_local std::mt19937 generator(std::random_device{}());
        static thread_local std::uniform_real_distribution<double> distribution(0.0, 1.0);
        
        return JSValue(distribution(generator));
    }
}
