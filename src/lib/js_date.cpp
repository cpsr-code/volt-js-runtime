#include "js_date.h"
#include "js_object.h"
#include "js_function.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace JSDate {

    // --- Date.now() ---
    JSValue now(const std::vector<JSValue>& args) {
        auto time_now = std::chrono::system_clock::now();
        auto duration = time_now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return JSValue(static_cast<double>(millis));
    }

    // --- new Date() ---
    JSValue create(const std::vector<JSValue>& args) {
        auto dateObj = JSObject::create();

        // 1. Determine the time. If no args, use current time. 
        // (For the hackathon, we'll keep it simple and handle the default empty constructor).
        auto time_now = std::chrono::system_clock::now();
        auto duration = time_now.time_since_epoch();
        long long current_millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        // We store the internal time state in a shared_ptr so all closures can access/mutate it.
        auto internal_time = std::make_shared<long long>(current_millis);

        // 2. Attach Methods using JSFunction wrappers

        // d.getTime()
        dateObj->properties["getTime"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            return JSValue(static_cast<double>(*internal_time));
        }));

        // d.getFullYear()
        dateObj->properties["getFullYear"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_year + 1900));
        }));

        // d.getMonth() (0-11)
        dateObj->properties["getMonth"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_mon));
        }));

        // d.getDate() (1-31)
        dateObj->properties["getDate"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_mday));
        }));

        // d.getHours()
        dateObj->properties["getHours"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_hour));
        }));

        // d.getMinutes()
        dateObj->properties["getMinutes"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_min));
        }));

        // d.getSeconds()
        dateObj->properties["getSeconds"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::localtime(&t);
            return JSValue(static_cast<double>(tm->tm_sec));
        }));

        // d.toISOString()
        dateObj->properties["toISOString"] = JSValue(JSFunction::create([internal_time](const std::vector<JSValue>&, JSValue) {
            std::time_t t = *internal_time / 1000;
            std::tm* tm = std::gmtime(&t);
            char buffer[30];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", tm);
            return JSValue(std::string(buffer));
        }));

        // Return the Object wrapped inside a JSValue
        return JSValue(dateObj);
    }
}