#pragma once

#include "lexer.h"
#include "symexpr.h"
#include <stdexcept>
#include <type_traits>

template<typename Number = DefaultNumber>
class Parser {
    Lexer lexer;

public:
    Parser(std::string source) : lexer(std::move(source)) {}

    Expression<Number> parse() {
        auto expr = parse_sum();
        if (!lexer.peek().is(TOK_EOF)) {
            throw std::invalid_argument("Expected end of input");
        }
        return expr;
    }

private:
    Expression<Number> parse_sum() {
        auto left = parse_product();
        while (true) {
            if (lexer.peek().str() == "+") {
                lexer.consume();
                left = left + parse_product();
            } else if (lexer.peek().str() == "-") {
                lexer.consume();
                left = left - parse_product();
            } else {
                break;
            }
        }
        return left;
    }

    Expression<Number> parse_product() {
        auto left = parse_power();
        while (true) {
            if (lexer.peek().str() == "*") {
                lexer.consume();
                left = left * parse_power();
            } else if (lexer.peek().str() == "/") {
                lexer.consume();
                left = left / parse_power();
            } else {
                break;
            }
        }
        return left;
    }

    Expression<Number> parse_power() {
        auto left = parse_unary();
        if (lexer.peek().str() == "^") {
            lexer.consume();
            return pow(left, parse_power());
        }
        return left;
    }

    Expression<Number> parse_unary() {
        if (lexer.peek().str() == "-") {
            lexer.consume();
            return -parse_unary();
        }
        return parse_atom();
    }

    Expression<Number> parse_atom() {
        Token tok = lexer.peek();
        
        if (tok.is(TOK_NUMBER)) {
            lexer.consume();
            if constexpr (std::is_same_v<Number, complex>) {
                if (lexer.peek().is(TOK_NAME) && lexer.peek() == "i") {
                    lexer.consume();
                    return Expression<complex>(complex(0, 1) * tok.value());
                }
            }
            return Expression<Number>(tok.value());
        }
        
        if (tok.is(TOK_NAME)) {
            lexer.consume();
            std::string name(tok.str());
            
            if (lexer.peek().str() == "(") {
                lexer.consume();
                auto arg = parse_sum();
                if (lexer.peek().str() != ")") {
                    throw std::invalid_argument("Expected ')'");
                }
                lexer.consume();

                if (name == "sin") return sin(arg);
                if (name == "cos") return cos(arg);
                if (name == "ln") return ln(arg);
                if (name == "exp") return exp(arg);
                
                throw std::invalid_argument("Unknown function: " + name);
            }
    
            // Check for function names used without parentheses
            if (name == "sin" || name == "cos" || name == "ln" || name == "exp") {
                throw std::invalid_argument("Function '" + name + "' must have an argument");
            }
    
            if (name == "pi") {
                return Expression<Number>(M_PI);
            }
            if (name == "e") {
                return Expression<Number>(M_E);
            }
            if constexpr (std::is_same_v<Number, complex>) {
                if (name == "i") {
                    return Expression<complex>(complex(0, 1));
                }
            }
            
            return Expression<Number>::var(name);
        }
        
        if (tok.str() == "(") {
            lexer.consume();
            auto expr = parse_sum();
            if (lexer.peek().str() != ")") {
                throw std::invalid_argument("Expected ')'");
            }
            lexer.consume();
            return expr;
        }
        
        throw std::invalid_argument("Unexpected token: " + std::string(tok.str()));
    }
};

template<typename Number = DefaultNumber>
Expression<Number> parse(const std::string& source) {
    return Parser<Number>(source).parse();
}