CXX = g++
CXXFLAGS = -std=c++17 -O3 -I src/compiler -I src/lib

# Target executable
TARGET = volt

# Library
LIB = libvoltjs.a

# Source files
COMPILER_SRCS = src/compiler/lexer.cpp src/compiler/parser.cpp src/compiler/generator.cpp
LIB_SRCS = src/lib/js_value.cpp src/lib/js_array.cpp src/lib/js_object.cpp src/lib/js_function.cpp src/lib/js_math.cpp src/lib/js_date.cpp
MAIN_SRC = src/main.cpp

COMPILER_OBJS = $(COMPILER_SRCS:.cpp=.o)
LIB_OBJS = $(LIB_SRCS:.cpp=.o)
MAIN_OBJ = $(MAIN_SRC:.cpp=.o)

all: $(LIB) $(TARGET)

$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

$(TARGET): $(MAIN_OBJ) $(COMPILER_OBJS) $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_OBJ) $(COMPILER_OBJS) -L. -lvoltjs

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(COMPILER_OBJS) $(LIB_OBJS) $(MAIN_OBJ) $(TARGET) $(LIB) .temp_output.cpp .temp_exec.exe
