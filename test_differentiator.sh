#!/bin/bash

DIFFERENTIATOR="build/debug/differentiator"

assert_equals() {
    local expected="$1"
    local actual="$2"
    local test_name="$3"
    
    if [ "$actual" = "$expected" ]; then
        echo "✅ $test_name: PASS"
    else
        echo "❌ $test_name: FAIL"
        echo "Expected: $expected"
        echo "Got:      $actual"
    fi
}
echo "Testing expression evaluation..."
result=$($DIFFERENTIATOR --eval "x * y" "x=10" "y=12")
assert_equals "120" "$result" "Simple multiplication"

result=$($DIFFERENTIATOR --eval "x * sin(x)" "x=2")
assert_equals "1.81859" "$result" "Function with sin"

echo -e "\nTesting differentiation..."
result=$($DIFFERENTIATOR --diff "x * sin(x)" --by x)
assert_equals "x * cos(x) + sin(x)" "$result" "Derivative of x*sin(x)"

result=$($DIFFERENTIATOR --diff "x * x" --by x)
assert_equals "x + x" "$result" "Derivative of x^2"

result=$($DIFFERENTIATOR --diff "cos(x)" --by x)
assert_equals "-sin(x)" "$result" "Derivative of cos(x)"

result=$($DIFFERENTIATOR --eval "2 * x + 1" "x=3")
assert_equals "7" "$result" "Linear expression"

result=$($DIFFERENTIATOR --diff "exp(x)" --by x)
assert_equals "exp(x)" "$result" "Derivative of exp(x)"

result=$($DIFFERENTIATOR --eval "x / y" "x=10" "y=2")
assert_equals "5" "$result" "Division operation"

result=$($DIFFERENTIATOR --eval "3 + 4i * (1 + 2i)")
assert_equals "-5 + 4i" "$result" "Complex number multiplication"

result=$($DIFFERENTIATOR --eval "(10 + 5i) / (2 + i)")
assert_equals "5" "$result" "Complex number division"

result=$($DIFFERENTIATOR --eval "(3 + 4i) + (1 + 2i)")
assert_equals "4 + 6i" "$result" "Complex number addition"

result=$($DIFFERENTIATOR --eval "(3 + 4i) - (1 + 2i)")
assert_equals "2 + 2i" "$result" "Complex number subtraction"

result=$($DIFFERENTIATOR --eval "z * (2 + 3i)" "z=1+i")
assert_equals "-1 + 5i" "$result" "Complex substitution and multiplication"

result=$($DIFFERENTIATOR --eval "(x + y*i) * (3 + 4i)" "x=2" "y=1")
assert_equals "2 + 11i" "$result" "Complex expression with multiple substitutions"

result=$($DIFFERENTIATOR --eval "exp(pi * i)")
assert_equals "-1 + 1.22465e-16i" "$result" "Euler's identity"

result=$($DIFFERENTIATOR --eval "(a + b*i)^2" "a=1" "b=1")
assert_equals "1.22465e-16 + 2i" "$result" "Complex number squared"

result=$($DIFFERENTIATOR --diff "(x + y*i) * (1 - i)" --by x)
assert_equals "1 + -1i" "$result" "Complex derivative with respect to x"

result=$($DIFFERENTIATOR --diff "sin(x + y*i)" --by y)
assert_equals "cos(x + y * 1i) * 1i" "$result" "Complex derivative of sin"