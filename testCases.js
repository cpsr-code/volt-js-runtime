// testCases.js

// Test Case 1: Odd/Even Checker
{
    let num = 7;
    if (num % 2 === 0) {
        console.log(num + " is Even");
    } else {
        console.log(num + " is Odd");
    }
}

// Test Case 2: Triangle Pattern using For Loop
{
    for (let i = 1; i <= 5; i++) {
        let row = "";
        for (let j = 1; j <= i; j++) {
            row += "*";
        }
        console.log(row);
    }
}

// Test Case 3: Armstrong Number
{
    function isArmstrong(num) {
        let temp = num;
        let sum = 0;
        while (temp > 0) {
            let digit = temp % 10;
            sum += digit ** 3;
            temp = Math.floor(temp / 10);
        }
        return sum === num;
    }
    console.log(isArmstrong(153));
    console.log(isArmstrong(123));
}

// Test Case 4: Array Reverse
{
    let arr = [1, 2, 3, 4, 5];
    let reversed = [...arr].reverse();
    console.log("Original: " + arr.join(", "));
    console.log("Reversed: " + reversed.join(", "));
}

// Test Case 5: String Palindrome Check
{
    let str = "racecar";
    let reversed = str.split("").reverse().join("");
    if (str === reversed) {
        console.log(str + " is a Palindrome");
    } else {
        console.log(str + " is not a Palindrome");
    }
}