cmake_minimum_required(VERSION 3.10)
project(VoltJSRuntime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# List every .cpp file we wrote
set(SOURCES
    src/main.cpp
    src/lib/js_value.cpp
    src/lib/js_array.cpp
    src/lib/js_object.cpp
    src/lib/js_function.cpp
    src/lib/js_math.cpp
    src/lib/js_date.cpp
    src/compiler/lexer.cpp
    src/compiler/parser.cpp
    src/compiler/generator.cpp
)

# Create the transpiler executable
add_executable(js_runtime ${SOURCES})