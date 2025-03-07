#include"../src/symexpr.h"
#include <cassert>
#include <iostream>

void test_diff_simplification() {
    // Test case 1: d/dx(x) = 1
    auto x = Expression("x");
    std::cout << "d/dx(" << x << ") = " << x.diff("x") << std::endl;

    // Test case 2: d/dx(x^2) = 2x
    auto x_squared = x * x;
    std::cout << "d/dx(" << x_squared << ") = " << x_squared.diff("x") << std::endl;

    // Test case 3: d/dx(1*x) = 1
    auto one_times_x = Expression(1) * x;
    std::cout << "d/dx(" << one_times_x << ") = " << one_times_x.diff("x") << std::endl;

    // Test case 4: d/dx(x^1) = 1
    auto x_power_one = pow(x, Expression(1));
    std::cout << "d/dx(" << x_power_one << ") = " << x_power_one.diff("x") << std::endl;

    // Test case 5: d/dx(x + 0) = 1
    auto x_plus_zero = x + Expression(0);
    std::cout << "d/dx(" << x_plus_zero << ") = " << x_plus_zero.diff("x") << std::endl;
}

int main() {
    test_diff_simplification();
}