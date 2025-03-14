#pragma once

#include <charconv>
#include <stdexcept>
#include <string>
#include <string_view>

enum TokenKind {
    TOK_OP,
    TOK_NUMBER,
    TOK_NAME,
    TOK_EOF,
};

class Token {
    TokenKind _kind;
    std::string_view _str;

public:
    Token(const char* source) {
        while (source != nullptr && *source != 0 && std::isspace(*source)) {
            source++;
        }
        if (source == nullptr || *source == 0) {
            _kind = TOK_EOF;
            _str = std::string_view();
        } else if (std::isdigit(*source)) {
            char *end;
            std::strtod(source, &end);
            _kind = TOK_NUMBER;
            _str = std::string_view(source, end);
        } else if (std::isalpha(*source)) {
            auto end = source;
            while (*end != 0 && std::isalnum(*end)) {
                end++;
            }
            _kind = TOK_NAME;
            _str = std::string_view(source, end);
        } else {
            _kind = TOK_OP;
            _str = std::string_view(source, source + 1);
        }
    }
    
    TokenKind kind() const { return _kind; }
    const char* data() const { return _str.data(); }
    std::size_t size() const { return _str.size(); }

    std::string_view str() const { return _str; }
    bool is(TokenKind k) const { return kind() == k; }

    double value() const {
        if (!is(TOK_NUMBER)) throw std::runtime_error("Not a number token");
        double result;
        std::from_chars(data(), data() + size(), result);
        return result;
    }

    bool operator==(const std::string_view& other) const { return _str == other; }
};

class Lexer {
    std::string source;
    Token current;
    Token next;
public:
    Lexer(std::string source_)
        : source(std::move(source_)),
        current(source.c_str()),
        next(current.data() + current.size())
    {}

    const Token& peek() const { return current; }
    const Token& peek2() const { return next; }

    void consume() {
        current = next;
        next = Token(current.data() + current.size());
    }
};

