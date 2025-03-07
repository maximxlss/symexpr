#include"utils.h"
#include"../src/symexpr.h"
#include <stdexcept>

void test_basic_numbers() {
    assert_eq(Expression("1").eval(), 1);
    assert_eq(Expression("-1").eval(), -1);
    assert_eq(Expression("1.5").eval(), 1.5);

    assert_eq(Expression<complex>("1").eval(), complex(1, 0));
    assert_eq(Expression<complex>("1i").eval(), complex(0, 1));
    assert_eq(Expression<complex>("1.5i").eval(), complex(0, 1.5));

    assert_eq(Expression("1").subs("a", 0).eval(), 1);
    auto a = Expression("a");
    assert_eq(a.subs("a", 1.0).eval(), 1.0);
    assert_throws<std::invalid_argument>([&]() {
        a.eval();
    });
    assert_throws<std::invalid_argument>([&]() {
        a.subs("b", 1.0).eval();
    });
    auto b = a.subs("a", Expression("b"));
    assert_eq(b.subs("b", 1.0).eval(), 1.0);
    assert_throws<std::invalid_argument>([&]() {
        b.eval();
    });
    assert_throws<std::invalid_argument>([&]() {
        b.subs("a", 1.0).eval();
    });

    assert_eq(Expression(0), Expression(0));
    assert_eq(Expression("a"), Expression("a"));
}

void test_basic_addition() {
    assert_eq((Expression(1) + Expression(1)).eval(), 2);
    assert_eq((Expression("a") + Expression(1)).subs("a", 1).eval(), 2);
    assert_eq((Expression("a") + Expression("b")).subs("a", 1).subs("b", 2).eval(), 3);
    assert_eq((Expression("a") + Expression("a")).subs("a", 1).eval(), 2);
    assert_eq((Expression("a") + Expression("a")).subs("a", Expression("b")).subs("b", 1).eval(), 2);

    assert_eq((Expression<complex>("1i") + Expression<complex>("1")).eval(), complex(1, 1));
    assert_eq((Expression<complex>("1i") + Expression<complex>("1i")).eval(), complex(0, 2));
    assert_eq((Expression<complex>("1") + Expression<complex>("1")).eval(), complex(2, 0));

    assert_throws<std::invalid_argument>([&]() {
        (Expression("a") + Expression(1)).eval();
    });
    assert_throws<std::invalid_argument>([&]() {
        (Expression("a") + Expression("b")).subs("a", 1).eval();
    });

    assert_eq((Expression("a") + Expression("b") + Expression("c")).subs("a", 1).subs("b", 2).subs("c", 3).eval(), 6);
    assert_eq((Expression("a") + Expression("a") + Expression("a")).subs("a", 1).eval(), 3);
    assert_eq((Expression("a") + Expression("b") + Expression("a")).subs("a", 1).subs("b", 2).eval(), 4);
    assert_eq((Expression("a") + Expression("b") + Expression("c")).subs("a", Expression("x")).subs("b", Expression("y")).subs("c", Expression("z")).subs("x", 1).subs("y", 2).subs("z", 3).eval(), 6);
    assert_eq((Expression<complex>("1i") + Expression<complex>("1") + Expression<complex>("1i")).eval(), complex(1, 2));    
}

void test_basic_negation() {
    assert_eq((-Expression(1)).eval(), -1);
    assert_eq((-Expression(-1)).eval(), 1);
    assert_eq((-Expression("a")).subs("a", 1).eval(), -1);
    assert_eq((-Expression("a")).subs("a", -1).eval(), 1);

    assert_eq((-Expression<complex>("1i")).eval(), complex(0, -1));
    assert_eq((-Expression<complex>("-1i")).eval(), complex(0, 1));
    assert_eq((-Expression<complex>("1")).eval(), complex(-1, 0));
    assert_eq((-Expression<complex>("-1")).eval(), complex(1, 0));

    assert_eq((Expression(1) - Expression(1)).eval(), 0);
    assert_eq((Expression("a") - Expression(1)).subs("a", 1).eval(), 0);
    assert_eq((Expression("a") - Expression("b")).subs("a", 3).subs("b", 2).eval(), 1);
    assert_eq((Expression("a") - Expression("a")).subs("a", 1).eval(), 0);

    assert_eq((Expression<complex>("1i") - Expression<complex>("1")).eval(), complex(-1, 1));
    assert_eq((Expression<complex>("1i") - Expression<complex>("1i")).eval(), complex(0, 0));
    assert_eq((Expression<complex>("1") - Expression<complex>("1")).eval(), complex(0, 0));
}

void test_basic_multiplication() {
    assert_eq((Expression(2) * Expression(3)).eval(), 6);
    assert_eq((Expression("a") * Expression(2)).subs("a", 3).eval(), 6);
    assert_eq((Expression("a") * Expression("b")).subs("a", 2).subs("b", 3).eval(), 6);
    assert_eq((Expression("a") * Expression("a")).subs("a", 2).eval(), 4);
    assert_eq((Expression("a") * Expression("a")).subs("a", Expression("b")).subs("b", 2).eval(), 4);

    assert_eq((Expression<complex>("1i") * Expression<complex>("1")).eval(), complex(0, 1));
    assert_eq((Expression<complex>("1i") * Expression<complex>("1i")).eval(), complex(-1, 0));
    assert_eq((Expression<complex>("1") * Expression<complex>("1")).eval(), complex(1, 0));

    assert_eq((Expression("a") * Expression("b") * Expression("c")).subs("a", 2).subs("b", 3).subs("c", 4).eval(), 24);
    assert_eq((Expression("a") * Expression("a") * Expression("a")).subs("a", 2).eval(), 8);
    assert_eq((Expression("a") * Expression("b") * Expression("a")).subs("a", 2).subs("b", 3).eval(), 12);
    assert_eq((Expression("a") * Expression("b") * Expression("c")).subs("a", Expression("x")).subs("b", Expression("y")).subs("c", Expression("z")).subs("x", 2).subs("y", 3).subs("z", 4).eval(), 24);
    assert_eq((Expression<complex>("1i") * Expression<complex>("1") * Expression<complex>("1i")).eval(), complex(-1, 0));
}

void test_basic_division() {
    assert_eq((Expression(6) / Expression(3)).eval(), 2);
    assert_eq((Expression("a") / Expression(2)).subs("a", 6).eval(), 3);
    assert_eq((Expression("a") / Expression("b")).subs("a", 6).subs("b", 3).eval(), 2);
    assert_eq((Expression("a") / Expression("a")).subs("a", 2).eval(), 1);
    assert_eq((Expression("a") / Expression("a")).subs("a", Expression("b")).subs("b", 2).eval(), 1);

    assert_eq((Expression<complex>("1i") / Expression<complex>("1")).eval(), complex(0, 1));
    assert_eq((Expression<complex>("1i") / Expression<complex>("1i")).eval(), complex(1, 0));
    assert_eq((Expression<complex>("1") / Expression<complex>("1")).eval(), complex(1, 0));

    assert_eq((Expression("a") / Expression("b") / Expression("c")).subs("a", 24).subs("b", 3).subs("c", 4).eval(), 2);
    assert_eq((Expression("a") / Expression("a") / Expression("a")).subs("a", 2).eval(), 0.5);
    assert_eq((Expression("a") / Expression("b") / Expression("a")).subs("a", 6).subs("b", 3).eval(), 1./3);
    assert_eq((Expression("a") / Expression("b") / Expression("c")).subs("a", Expression("x")).subs("b", Expression("y")).subs("c", Expression("z")).subs("x", 24).subs("y", 3).subs("z", 4).eval(), 2);
    assert_eq((Expression<complex>("1i") / Expression<complex>("1") / Expression<complex>("1i")).eval(), complex(1, 0));

    assert_throws<std::invalid_argument>([&]() {
        (Expression("a") / Expression(0)).eval();
    });
}

void test_basic_exponents() {
    assert_eq((Expression(2) ^ Expression(3)).eval(), 8);
    assert_eq((Expression("a") ^ Expression(2)).subs("a", 3).eval(), 9);
    assert_eq((Expression("a") ^ Expression("b")).subs("a", 2).subs("b", 3).eval(), 8);
    assert_eq((Expression("a") ^ Expression("a")).subs("a", 2).eval(), 4);
    assert_eq((Expression("a") ^ Expression("a")).subs("a", Expression("b")).subs("b", 2).eval(), 4);

    assert_eq((Expression<complex>("1i") ^ Expression<complex>("1")).eval(), complex(0, 1));
    assert_eq((Expression<complex>("1i") ^ Expression<complex>("2")).eval(), pow(complex(0, 1), complex(2, 0)));
    assert_eq((Expression<complex>("1") ^ Expression<complex>("2")).eval(), complex(1, 0));

    assert_eq((Expression("a") ^ Expression("b") ^ Expression("c")).subs("a", 2).subs("b", 3).subs("c", 2).eval(), 64);
    assert_eq((Expression("a") ^ Expression("a") ^ Expression("a")).subs("a", 2).eval(), 16);
    assert_eq((Expression("a") ^ Expression("b") ^ Expression("a")).subs("a", 2).subs("b", 3).eval(), 64);
    assert_eq((Expression("a") ^ Expression("b") ^ Expression("c")).subs("a", Expression("x")).subs("b", Expression("y")).subs("c", Expression("z")).subs("x", 2).subs("y", 3).subs("z", 2).eval(), 64);
}

void test_basic_functions() {
    assert_close(sin(Expression(0)).eval(), 0);
    assert_close(sin(Expression("a")).subs("a", 0).eval(), 0);
    assert_close(sin(Expression(3.14159265359/2)).eval(), 1);
    assert_close(sin(Expression("pi") / Expression(2)).subs("pi", 3.14159265359).eval(), 1);
    assert_close(sin(Expression<complex>("0")).eval(), complex(0,0));
    assert_close(sin(Expression<complex>("1i")).eval(), complex(0, sinh(1)));
    assert_close(sin(-Expression(3.14159265359/2)).eval(), -1);
    assert_close(sin(Expression("a") + Expression("b")).subs("a", 0).subs("b", 3.14159265359/2).eval(), 1);

    assert_close(cos(Expression(0)).eval(), 1);
    assert_close(cos(Expression("a")).subs("a", 0).eval(), 1);
    assert_close(cos(Expression(3.14159265359/2)).eval(), 0);
    assert_close(cos(Expression("pi") / Expression(2)).subs("pi", 3.14159265359).eval(), 0);
    assert_close(cos(Expression<complex>("0")).eval(), complex(1,0));
    assert_close(cos(Expression<complex>("1i")).eval(), complex(cosh(1), 0));

    assert_close(ln(Expression(1)).eval(), 0);
    assert_close(ln(Expression("a")).subs("a", 1).eval(), 0);
    assert_close(ln(Expression("e")).subs("e", 2.71828182846).eval(), 1);
    assert_close(ln(Expression<complex>("1")).eval(), complex(0,0));
    assert_close(ln(Expression<complex>("1i")).eval(), complex(0, 3.14159265359/2));

    assert_close(exp(Expression(0)).eval(), 1);
    assert_close(exp(Expression("a")).subs("a", 0).eval(), 1);
    assert_close(exp(Expression(1)).eval(), 2.71828182846);
    assert_close(exp(Expression<complex>("0")).eval(), complex(1,0));
    assert_close(exp(Expression<complex>("1i")).eval(), complex(cos(1), sin(1)));
}

void test_basic_to_string() {
    assert_eq(Expression("1").to_string(), "1");
    assert_eq(Expression("-1").to_string(), "-1");
    assert_eq(Expression("a").to_string(), "a");
    assert_eq((Expression("1") + Expression("2")).to_string(), "1 + 2");
    assert_eq((Expression("a") + Expression("b")).to_string(), "a + b");
    assert_eq((-Expression("1")).to_string(), "-1");
    assert_eq((-Expression("a")).to_string(), "-a");
    assert_eq((Expression("a") / Expression("b")).to_string(), "a / b");
    assert_eq((Expression("2") ^ Expression("3")).to_string(), "2 ^ 3");
    assert_eq(sin(Expression("x")).to_string(), "sin(x)");
    assert_eq(cos(Expression("x")).to_string(), "cos(x)");
    assert_eq(ln(Expression("x")).to_string(), "ln(x)");
    assert_eq(exp(Expression("x")).to_string(), "exp(x)");

    assert_eq((Expression("1") + Expression("2") + Expression("3")).to_string(), "1 + 2 + 3");
    assert_eq((Expression("a") * Expression("b") * Expression("c")).to_string(), "a * b * c");
    assert_eq((Expression("x") ^ Expression("y") ^ Expression("z")).to_string(), "x ^ y ^ z");
    assert_eq(sin(Expression("a") + Expression("b")).to_string(), "sin(a + b)");
    assert_eq((sin(Expression("x")) + cos(Expression("y"))).to_string(), "sin(x) + cos(y)");
    assert_eq((-(Expression(1) + Expression(1))).to_string(), "-(1 + 1)");
}

void test_basic_differentiation() {
    assert_eq(Expression("1").diff("x").eval(), 0);
    assert_eq(Expression("x").diff("x").eval(), 1);
    assert_eq(Expression("y").diff("x").eval(), 0);

    assert_eq((Expression("x") + Expression("y")).diff("x").subs("x", 2).subs("y", 3).eval(), 1);
    assert_eq((Expression("x") + Expression("x")).diff("x").subs("x", 2).eval(), 2);
    
    assert_eq((-Expression("x")).diff("x").subs("x", 2).eval(), -1);
    
    assert_eq((Expression("x") * Expression("y")).diff("x").subs("x", 2).subs("y", 3).eval(), 3);
    assert_eq((Expression("x") * Expression("x")).diff("x").subs("x", 2).eval(), 4);

    assert_eq((Expression("x") / Expression("y")).diff("x").subs("x", 2).subs("y", 3).eval(), 1.0/3.0);

    assert_eq(pow(Expression("x"), Expression("2")).diff("x").subs("x", 2).eval(), 4);

    assert_eq(sin(Expression("x")).diff("x").subs("x", 0).eval(), 1);
    assert_eq(cos(Expression("x")).diff("x").subs("x", 0).eval(), 0);
    assert_eq(ln(Expression("x")).diff("x").subs("x", 1).eval(), 1);
    assert_eq(exp(Expression("x")).diff("x").subs("x", 0).eval(), 1);
}

void test_symbolic_differentiation() {
    assert_eq(Expression("1").diff("x").to_string(), "0");
    assert_eq(Expression("x").diff("x").to_string(), "1");
    assert_eq(Expression("y").diff("x").to_string(), "0");

    assert_eq((Expression("x") + Expression("y")).diff("x").to_string(), "1");
    assert_eq((Expression("x") + Expression("x")).diff("x").to_string(), "1 + 1");
    
    assert_eq((-Expression("x")).diff("x").to_string(), "-1");
    
    assert_eq((Expression("x") * Expression("y")).diff("x").to_string(), "y");
    assert_eq((Expression("x") * Expression("x")).diff("x").to_string(), "x + x");

    assert_eq((Expression("x") / Expression("y")).diff("x").to_string(), "y / (y * y)");

    assert_eq((Expression("x") ^ Expression("2")).diff("x").to_string(), "x ^ 2 * 2 / x");

    assert_eq(sin(Expression("x")).diff("x").to_string(), "cos(x)");
    assert_eq(cos(Expression("x")).diff("x").to_string(), "-sin(x)");
    assert_eq(ln(Expression("x")).diff("x").to_string(), "1 / x");
    assert_eq(exp(Expression("x")).diff("x").to_string(), "exp(x)");
}

int main() {
    test_basic_numbers();
    test_basic_addition();
    test_basic_negation();
    test_basic_multiplication();
    test_basic_division();
    test_basic_exponents();
    test_basic_functions();
    test_basic_to_string();
    test_basic_differentiation();
    test_symbolic_differentiation();
    summary();
}
