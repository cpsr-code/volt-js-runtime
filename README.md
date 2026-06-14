# Volt JS Runtime

Volt-JS-Runtime is a highly-optimized, C++ based JavaScript runtime. It behaves as a transpiler engine, instantly converting JavaScript ASTs into high-performance C++ and executing them dynamically.

##  Quick Start & Installation

To run Volt, you need a C++17 compatible compiler (`g++`) installed on your system.

**1. Clone the repository**
```bash
git clone https://github.com/cpsr-code/volt-js-runtime.git
cd volt-js-runtime
```

**2. Build the engine**
Run the build script to compile the standard library and generate the `volt` binary:
- **Windows (PowerShell):**
  ```powershell
  .\build.ps1
  ```
- **Linux / macOS:**
  ```bash
  make
  ```

**3. Run JavaScript!**
You can execute JavaScript by passing your `.js` file to the compiled runtime:

```bash
./volt my_script.js
```

---

## Supported Features

Volt natively supports standard JavaScript syntax and behaviors, including:
- **Variables:** `let`, `const`, and `var`.
- **Data Types:** Numbers, Booleans, Strings, Arrays, and Objects.
- **Control Flow:** `if`, `else`, `switch`, `for`, `while`, `do...while`, `break`, `continue`, `return`.
- **Functions:** Standard functions, arrow functions, and first-class closures/callbacks.
- **Modern Syntax:** Spread and Rest operators (`...`).
- **Built-in Objects:** 
  - `Math` (e.g., `Math.random()`, `Math.floor()`)
  - `Date` (e.g., `Date.now()`, `new Date()`)
- **Array Methods:** `map()`, `filter()`, `reduce()`, `find()`, `some()`, `every()`, `push()`, `pop()`, `reverse()`, and more.
- **String Methods:** `split()`, `slice()`, `indexOf()`, `toUpperCase()`, etc.
- **Type Coercion:** Standard JS loose equality (`==`) and strict equality (`===`).

---

## What Makes Volt Different?

Volt works fundamentally differently from traditional interpreters like V8 or SpiderMonkey:

1. **AST to C++ Transpilation:** Instead of interpreting bytecode through a virtual machine, Volt reads your `.js` file, builds an Abstract Syntax Tree (AST), and directly transpiles it into modern `C++17`. 
2. **Native Execution Speeds:** After transpiling, Volt automatically compiles your code against a prebuilt static library (`libvoltjs.a`) and runs it as a true native binary.
3. **O(1) Immutable Strings:** Real JavaScript string referencing under the hood using C++ `std::shared_ptr`.
4. **Heap-Optimized Iterators:** Array operations like `map` and `filter` are internally pre-allocated to bypass thousands of heap allocations, meaning raw execution performance can rival or exceed traditional engines for math-heavy loops.
