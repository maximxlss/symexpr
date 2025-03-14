#include"../src/symexpr.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// > differentiator --eval “x * y“ x=10 y=12
// 120

// > differentiator --diff “x * sin(x)“ --by x
// x * cos(x) + sin(x)

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  differentiator --eval EXPR [VAR=VALUE...]\n";
    std::cout << "  differentiator --diff EXPR --by VAR\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_usage();
        return 1;
    }

    std::string op = argv[1];
    std::string expr_str = argv[2];

    if (op == "--eval") {
        std::vector<std::pair<std::string, complex>> values_map;
        for (int i = 3; i < argc; i++) {
            std::string arg = argv[i];
            size_t eq_pos = arg.find('=');
            if (eq_pos == std::string::npos) {
                print_usage();
                return 1;
            }
            std::string var = arg.substr(0, eq_pos);
            complex val = Expression<complex>(arg.substr(eq_pos + 1)).eval();
            values_map.emplace_back(var, val);
        }
        if (std::all_of(values_map.begin(), values_map.end(), [](std::pair<std::string, complex> v) {
            return v.second.imag() == 0;
        })) {
            try {
                Expression<double> expr(expr_str);
                for (auto [var, val]: values_map) {
                    expr = expr.subs(var, val.real());
                }
                std::cout << expr.eval() << std::endl;
            } catch (const std::invalid_argument& e) {
                if (std::string(e.what()).find("Expected end of input") != std::string::npos) {
                    Expression<complex> expr(expr_str);
                    for (auto [var, val]: values_map) {
                        expr = expr.subs(var, val);
                    }
                    complex val = expr.eval();
                    std::cout << val.real() << " + " << val.imag() << "i" << std::endl;
                }
            }
        } else {
            Expression<complex> expr(expr_str);
            for (auto [var, val]: values_map) {
                expr = expr.subs(var, val);
            }
            complex val = expr.eval();
            std::cout << val.real() << " + " << val.imag() << "i" << std::endl;
        }
        
    }
    else if (op == "--diff" && argc == 5 && std::string(argv[3]) == "--by") {
        Expression<complex> expr(expr_str);
        std::string var = argv[4];
        std::cout << expr.diff(var).to_string() << std::endl;
    }
    else {
        print_usage();
        return 1;
    }

    return 0;
}
