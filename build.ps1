Write-Host "Building libvoltjs.a static library..."
g++ -std=c++17 -O3 -c src/lib/js_value.cpp -o src/lib/js_value.o -I src/lib
g++ -std=c++17 -O3 -c src/lib/js_array.cpp -o src/lib/js_array.o -I src/lib
g++ -std=c++17 -O3 -c src/lib/js_object.cpp -o src/lib/js_object.o -I src/lib
g++ -std=c++17 -O3 -c src/lib/js_function.cpp -o src/lib/js_function.o -I src/lib
g++ -std=c++17 -O3 -c src/lib/js_math.cpp -o src/lib/js_math.o -I src/lib
g++ -std=c++17 -O3 -c src/lib/js_date.cpp -o src/lib/js_date.o -I src/lib
ar rcs libvoltjs.a src/lib/js_value.o src/lib/js_array.o src/lib/js_object.o src/lib/js_function.o src/lib/js_math.o src/lib/js_date.o

Write-Host "Building volt.exe..."
g++ -std=c++17 -O3 src/main.cpp src/compiler/lexer.cpp src/compiler/parser.cpp src/compiler/generator.cpp -o volt.exe -I src/compiler -I src/lib -L. -lvoltjs

Write-Host "Build complete! You can now run volt.exe"
