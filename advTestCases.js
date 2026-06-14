// advTestCases.js
// Advance test cases covering: let/const, operators, conditionals, loops,
// arrays (push, pop, shift, unshift, slice, splice, concat, includes, indexOf, sort, reverse),
// strings (replace, replaceAll, substring, slice, split, trim, toUpperCase, toLowerCase, includes, startsWith, endsWith, indexOf),
// objects, functions (declaration, expression, arrow), callbacks, map/filter/reduce/find/some/every,
// Math, Date, type coercion, spread/rest, and more.

// ==============================
// Test Case H1: Advanced Array & String Operations
// ==============================
{
    console.log("=== H1: Advanced Array & String Operations ===");

    let arr = [10, 20, 30, 40, 50];
    // push, pop, shift, unshift
    arr.push(60);
    arr.unshift(5);
    console.log("After push/unshift:", arr.join());  // 5,10,20,30,40,50,60
    arr.pop();
    arr.shift();
    console.log("After pop/shift:", arr.join());     // 10,20,30,40,50

    // splice and slice
    let spliced = arr.splice(2, 2, 35, 45);
    console.log("Spliced out:", spliced.join());     // 30,40
    console.log("After splice:", arr.join());        // 10,20,35,45,50

    let sliced = arr.slice(1, 4);
    console.log("Sliced (1-4):", sliced.join());     // 20,35,45

    // concat, includes, indexOf
    let other = [100, 200];
    let combined = arr.concat(other);
    console.log("Combined includes 35?", combined.includes(35));         // true
    console.log("Index of 45:", combined.indexOf(45));                   // 3

    // sort and reverse
    let unsorted = [3, 1, 4, 1, 5];
    console.log("Sorted:", unsorted.sort((a,b)=>a-b).join());             // 1,1,3,4,5
    console.log("Reversed:", unsorted.reverse().join());                 // 5,4,3,1,1

    // String advanced: replace, replaceAll, substring, slice, split, trim, case, includes, startsWith, endsWith
    let str = "  Hello, World! Hello, JS.  ";
    let trimmed = str.trim();
    console.log("Trimmed:", trimmed);                                    // "Hello, World! Hello, JS."
    console.log("Replace first Hello:", trimmed.replace("Hello", "Hi")); // "Hi, World! Hello, JS."
    console.log("ReplaceAll Hello:", trimmed.replaceAll("Hello", "Hi")); // "Hi, World! Hi, JS."
    console.log("Substring 7-12:", trimmed.substring(7, 12));            // "World"
    console.log("Slice 7-12:", trimmed.slice(7, 12));                    // "World"
    console.log("Split by ,:", trimmed.split(","));                      // ["Hello", " World! Hello", " JS."]
    console.log("Uppercase:", trimmed.toUpperCase());                    // "HELLO, WORLD! HELLO, JS."
    console.log("Lowercase:", trimmed.toLowerCase());                    // "hello, world! hello, js."
    console.log("Includes 'JS':", trimmed.includes("JS"));               // true
    console.log("Starts with 'Hello':", trimmed.startsWith("Hello"));    // true
    console.log("Ends with 'JS.':", trimmed.endsWith("JS."));            // true
    console.log("IndexOf 'World':", trimmed.indexOf("World"));           // 7
}

// ==============================
// Test Case H2: Objects, Functions, Callbacks, and this
// ==============================
{
    console.log("\n=== H2: Objects, Functions, Callbacks, and this ===");

    // object with methods and computed properties
    const key = "age";
    let person = {
        name: "Alice",
        [key]: 25,
        greet() {
            return `Hi, I'm ${this.name}`;
        },
        arrowGreet: () => `Hi, I'm ${this.name}`  // arrow function has its own this (global/undefined)
    };
    console.log("Person age:", person.age);                    // 25
    console.log("Greet method:", person.greet());              // Hi, I'm Alice
    console.log("Arrow greet (this issue):", person.arrowGreet()); // "Hi, I'm undefined" (or empty)

    // function declaration, expression, arrow
    function addDecl(a, b) { return a + b; }
    const addExpr = function(a, b) { return a + b; };
    const addArrow = (a, b) => a + b;
    console.log("Add (decl):", addDecl(5,3));                  // 8
    console.log("Add (expr):", addExpr(5,3));                  // 8
    console.log("Add (arrow):", addArrow(5,3));                // 8

    // callback example: forEach with thisArg
    let numbers = [1,2,3];
    let multiplier = {
        factor: 2,
        multiply(x) { return x * this.factor; }
    };
    let results = [];
    numbers.forEach(function(num) {
        results.push(this.multiply(num));
    }, multiplier);
    console.log("Callback with thisArg:", results);            // [2,4,6]
}

// ==============================
// Test Case H3: Array Methods (map, filter, reduce, find, some, every)
// ==============================
{
    console.log("\n=== H3: Array Methods (map/filter/reduce/find/some/every) ===");

    let data = [5, 12, 8, 130, 44];
    let mapped = data.map(x => x * 2);
    console.log("Map (*2):", mapped);                          // [10,24,16,260,88]

    let filtered = data.filter(x => x > 10);
    console.log("Filter (>10):", filtered);                    // [12,130,44]

    let reduced = data.reduce((acc, cur) => acc + cur, 0);
    console.log("Reduce sum:", reduced);                       // 199

    let found = data.find(x => x > 100);
    console.log("Find first >100:", found);                    // 130

    let someLarge = data.some(x => x > 100);
    console.log("Some >100:", someLarge);                      // true

    let allPositive = data.every(x => x > 0);
    console.log("Every >0:", allPositive);                     // true
}

// ==============================
// Test Case H4: Math, Date, Type Coercion, Spread/Rest
// ==============================
{
    console.log("\n=== H4: Math, Date, Type Coercion, Spread/Rest ===");

    // Math object
    console.log("Math.abs(-5):", Math.abs(-5));                // 5
    console.log("Math.pow(2,3):", Math.pow(2,3));              // 8
    console.log("Math.max(1,5,3):", Math.max(1,5,3));          // 5
    console.log("Math.floor(3.9):", Math.floor(3.9));          // 3
    console.log("Math.random() between 0-1:", (Math.random() >= 0 && Math.random() <= 1)); // true

    // Date object (just basic operations)
    let now = new Date();
    console.log("Date year:", now.getFullYear());              // e.g., 2026 (valid year)
    console.log("Date timestamp > 0:", now.getTime() > 0);     // true

    // Type coercion (loose equality, string to number, etc.)
    console.log("'5' - 3 =", '5' - 3);                        // 2 (coercion to number)
    console.log("'5' + 3 =", '5' + 3);                        // "53" (coercion to string)
    console.log("5 == '5':", 5 == '5');                       // true
    console.log("5 === '5':", 5 === '5');                     // false

    // Spread operator with arrays and objects
    let arr1 = [1,2,3];
    let arr2 = [4,5,6];
    let combinedSpread = [...arr1, ...arr2];
    console.log("Spread arrays:", combinedSpread);            // [1,2,3,4,5,6]

    let obj1 = { a:1, b:2 };
    let obj2 = { c:3, d:4 };
    let mergedObj = { ...obj1, ...obj2 };
    console.log("Spread objects:", mergedObj);                // {a:1,b:2,c:3,d:4}

    // Rest parameters
    function sumAll(...nums) {
        return nums.reduce((acc, n) => acc + n, 0);
    }
    console.log("Rest sum (1,2,3,4):", sumAll(1,2,3,4));      // 10
}

// ==============================
// Test Case H5: Nested Conditionals, Loops, and Edge Cases
// ==============================
{
    console.log("\n=== H5: Nested Conditionals, Loops, and Edge Cases ===");

    // FizzBuzz with nested conditionals
    for (let i = 1; i <= 15; i++) {
        if (i % 3 === 0 && i % 5 === 0) console.log("FizzBuzz");
        else if (i % 3 === 0) console.log("Fizz");
        else if (i % 5 === 0) console.log("Buzz");
        else console.log(i);
    }
    // Expected: 1,2,Fizz,4,Buzz,Fizz,7,8,Fizz,Buzz,11,Fizz,13,14,FizzBuzz

    // while loop with break and continue
    let count = 0;
    let output = [];
    while (count < 10) {
        count++;
        if (count % 2 === 0) continue;
        if (count > 7) break;
        output.push(count);
    }
    console.log("While loop (odd numbers <8):", output);      // [1,3,5,7]

    // do...while
    let j = 0;
    do {
        j++;
    } while (j < 0);
    console.log("do-while runs at least once:", j);           // 1

    // Edge: undefined, null, typeof
    let undefVar;
    let nullVar = null;
    console.log("typeof undefined:", typeof undefVar);         // "undefined"
    console.log("typeof null:", typeof nullVar);               // "object" (historical)
    console.log("null == undefined:", null == undefined);      // true
    console.log("null === undefined:", null === undefined);    // false
}

// ==============================
// Test Case H6: Complex Combination (String + Array + Function + Spread)
// ==============================
{
    console.log("\n=== H6: Complex Combination ===");

    // Transform a sentence: capitalize first letter of each word, reverse words order
    let sentence = "hello world from javascript";
    let transformed = sentence
        .split(" ")
        .map(word => word[0].toUpperCase() + word.slice(1))
        .reverse()
        .join(" ");
    console.log("Transformed sentence:", transformed);         // "Javascript From World Hello"

    // Simulate a pipe using reduce and functions
    const pipe = (...fns) => (x) => fns.reduce((v, f) => f(v), x);
    const add2 = x => x + 2;
    const multiply3 = x => x * 3;
    const subtract5 = x => x - 5;
    let result = pipe(add2, multiply3, subtract5)(10);
    console.log("Pipe (10 -> +2 -> *3 -> -5):", result);       // (10+2)=12*3=36-5=31

    // Object with getter/setter (advanced, but good to test)
    let obj = {
        _value: 0,
        get value() { return this._value; },
        set value(v) { this._value = v * 2; }
    };
    obj.value = 5;
    console.log("Getter/setter value:", obj.value);            // 10
}