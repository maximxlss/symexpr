#pragma once

#include <string>
#include <complex>
#include <algorithm>
#include <memory>
#include <format>
#include <optional>
#include <stdexcept>
#include <sstream>

using DefaultNumber = double;
using complex = std::complex<double>;

template<typename Number = DefaultNumber>
Number parse_number(const std::string& str);

template<>
inline double parse_number<double>(const std::string& str) {
    std::size_t processed;
    double result = std::stod(str, &processed);
    if (processed != str.size()) {
        throw std::invalid_argument(std::format("Invalid number `{}`", str));
    }
    return result;
}

template<>
inline complex parse_number<complex>(const std::string& str) {
    if (str.empty()) {
        return 0;
    }
    std::size_t processed;
    complex result = std::stod(str, &processed);
    bool is_imag = str.back() == 'i';
    if (processed != str.size() - is_imag) {
        throw std::invalid_argument(std::format("Invalid number `{}`", str));
    }
    if (is_imag) {
        result *= complex(0, 1);
    }
    return result;
}

template<typename Number>
struct Expression;

// Expr shall be stored in a shared_ptr and not be modified
template<typename Number = DefaultNumber>
struct Expr {
    // substitute and return the new value. return nullopt if unchanged.
    virtual std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const = 0;

    // evaluate into a Number.
    virtual Number eval() const = 0;

    virtual Expression<Number> diff(const std::string& name) const = 0;

    virtual std::string to_string() const = 0;

    virtual int precedence() const = 0;

    virtual bool operator==(const Expression<Number>&) const = 0;

    virtual ~Expr() = default;
};

template<typename Number = DefaultNumber>
struct NumExpr : Expr<Number> {
    Number value;

    NumExpr(Number _value) : value(_value) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        return {};
    };
    Number eval() const override {
        return value;
    };
    Expression<Number> diff(const std::string& name) const override {
        return Expression<Number>(Number(0));
    }
    std::string to_string() const override {
        if constexpr (std::is_same_v<Number, complex>) {
            std::stringstream ss;
            if (value.real() == 0) {
                ss << value.imag() << "i";
            } else if (value.imag() == 0) {
                ss << value.real();
            } else {
                ss << "(" << value.real() << " + " << value.imag() << "i)";
            }
            return ss.str();
        } else {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }
    }
    int precedence() const override {
        return 4;
    }
    bool operator==(const Expression<Number>& other) const override {
        NumExpr<Number>* v;
        return (v = dynamic_cast<NumExpr<Number>*>(other.inner.get())) && v->value == value;
    }
};

template<typename Number = DefaultNumber>
struct VarExpr : Expr<Number> {
    std::string name;

    VarExpr(const std::string& _name) : name(_name) {}

    std::optional<Expression<Number>> subs(const std::string& _name, const Expression<Number>& value) const override {
        if (name == _name) {
            return value;
        } else {
            return {};
        }
    };
    Number eval() const override {
        throw std::invalid_argument(std::format("Can't evaluate an unknown `{}`", name));
    };
    Expression<Number> diff(const std::string& name) const override {
        return Expression<Number>(name == this->name ? Number(1) : Number(0));
    }
    std::string to_string() const override{
        return name;
    }
    int precedence() const override {
        return 4;
    }
    bool operator==(const Expression<Number>& other) const override {
        VarExpr<Number>* v;
        return (v = dynamic_cast<VarExpr<Number>*>(other.inner.get())) && v->name == name;
    }
};

template<typename Number = DefaultNumber>
struct SumExpr : Expr<Number> {
    Expression<Number> lhs;
    Expression<Number> rhs;

    SumExpr(Expression<Number> _lhs, Expression<Number> _rhs) : lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        auto lhs_ = lhs.subs_maybe(name, value);
        auto rhs_ = rhs.subs_maybe(name, value);
        if (lhs_ || rhs_) {
            return lhs_.value_or(lhs) + rhs_.value_or(rhs);
        } else {
            return {};
        }
    };
    Number eval() const override {
        return lhs.eval() + rhs.eval();
    };
    Expression<Number> diff(const std::string& name) const override {
        return lhs.diff(name) + rhs.diff(name);
    }
    std::string to_string() const override {
        return std::format("{} + {}", 
            lhs.precedence() < this->precedence() ? "(" + lhs.to_string() + ")" : lhs.to_string(),
            rhs.precedence() < this->precedence() ? "(" + rhs.to_string() + ")" : rhs.to_string()
        );
    }
    int precedence() const override {
        return 0;
    }
    bool operator==(const Expression<Number>& other) const override {
        SumExpr<Number>* v;
        return (v = dynamic_cast<SumExpr<Number>*>(other.inner.get())) && v->lhs == lhs && v->rhs == rhs;
    }
};

template<typename Number = DefaultNumber>
struct NegExpr : Expr<Number> {
    Expression<Number> expr;

    NegExpr(Expression<Number> _expr) : expr(std::move(_expr)) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        return expr.subs_maybe(name, value).transform([](auto v){ return -v; });
    };
    Number eval() const override {
        return -expr.eval();
    };
    Expression<Number> diff(const std::string& name) const override {
        return -expr.diff(name);
    }
    std::string to_string() const override {
        return "-" + (expr.precedence() < this->precedence() ? "(" + expr.to_string() + ")" : expr.to_string());
    }
    int precedence() const override {
        return 4;
    }
    bool operator==(const Expression<Number>& other) const override {
        NegExpr<Number>* v;
        return (v = dynamic_cast<NegExpr<Number>*>(other.inner.get())) && v->expr == expr;
    }
};

template<typename Number = DefaultNumber>
struct MulExpr : Expr<Number> {
    Expression<Number> lhs;
    Expression<Number> rhs;

    MulExpr(Expression<Number> _lhs, Expression<Number> _rhs) : lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        auto lhs_ = lhs.subs_maybe(name, value);
        auto rhs_ = rhs.subs_maybe(name, value);
        if (lhs_ || rhs_) {
            return lhs_.value_or(lhs) * rhs_.value_or(rhs);
        } else {
            return {};
        }
    };
    Number eval() const override {
        return lhs.eval() * rhs.eval();
    };
    Expression<Number> diff(const std::string& name) const override {
        return lhs * rhs.diff(name) + rhs * lhs.diff(name);
    }
    std::string to_string() const override {
        return std::format("{} * {}", 
            lhs.precedence() < this->precedence() ? "(" + lhs.to_string() + ")" : lhs.to_string(),
            rhs.precedence() < this->precedence() ? "(" + rhs.to_string() + ")" : rhs.to_string()
        );
    }
    int precedence() const override {
        return 1;
    }
    bool operator==(const Expression<Number>& other) const override {
        MulExpr<Number>* v;
        return (v = dynamic_cast<MulExpr<Number>*>(other.inner.get())) && v->lhs == lhs && v->rhs == rhs;
    }
};

template<typename Number = DefaultNumber>
struct DivExpr : Expr<Number> {
    Expression<Number> lhs;
    Expression<Number> rhs;

    DivExpr(Expression<Number> _lhs, Expression<Number> _rhs) : lhs(std::move(_lhs)), rhs(std::move(_rhs)) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        auto lhs_ = lhs.subs_maybe(name, value);
        auto rhs_ = rhs.subs_maybe(name, value);
        if (lhs_ || rhs_) {
            return lhs_.value_or(lhs) / rhs_.value_or(rhs);
        } else {
            return {};
        }
    };
    Number eval() const override {
        return lhs.eval() / rhs.eval();
    };
    Expression<Number> diff(const std::string& name) const override {
        return (rhs * lhs.diff(name) - lhs * rhs.diff(name)) / (rhs * rhs);
    }
    std::string to_string() const override {
        return std::format("{} / {}", 
            lhs.precedence() < this->precedence() ? "(" + lhs.to_string() + ")" : lhs.to_string(),
            rhs.precedence() < this->precedence() ? "(" + rhs.to_string() + ")" : rhs.to_string()
        );
    }
    int precedence() const override {
        return 2;
    }
    bool operator==(const Expression<Number>& other) const override {
        DivExpr<Number>* v;
        return (v = dynamic_cast<DivExpr<Number>*>(other.inner.get())) && v->lhs == lhs && v->rhs == rhs;
    }
};

template<typename Number = DefaultNumber>
struct PowExpr : Expr<Number> {
    Expression<Number> base;
    Expression<Number> exponent;

    PowExpr(Expression<Number> _base, Expression<Number> _exponent) : base(std::move(_base)), exponent(std::move(_exponent)) {}

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        auto base_ = base.subs_maybe(name, value);
        auto exponent_ = exponent.subs_maybe(name, value);
        if (base_ || exponent_) {
            return pow(base_.value_or(base), exponent_.value_or(exponent));
        } else {
            return {};
        }
    };
    Number eval() const override {
        using std::pow;
        return pow(base.eval(), exponent.eval());
    };
    Expression<Number> diff(const std::string& name) const override {
        // Using the formula: d/dx(f^g) = f^g * (g*f'/f + g'*ln(f))
        return pow(base, exponent) * (exponent * base.diff(name) / base + exponent.diff(name) * ln(base));
    }
    std::string to_string() const override {
        return std::format("{} ^ {}", 
            base.precedence() < this->precedence() ? "(" + base.to_string() + ")" : base.to_string(),
            exponent.precedence() < this->precedence() ? "(" + exponent.to_string() + ")" : exponent.to_string()
        );
    }
    int precedence() const override {
        return 3;
    }
    bool operator==(const Expression<Number>& other) const override {
        PowExpr<Number>* v;
        return (v = dynamic_cast<PowExpr<Number>*>(other.inner.get())) && v->base == base && v->exponent == exponent;
    }
};

template<typename Number = DefaultNumber>
struct FunExpr : Expr<Number> {
    Expression<Number> expr;

    FunExpr(Expression<Number> _expr) : expr(std::move(_expr)) {}
};

template<typename Number, template<typename> typename Self>
struct FunExprImpl : FunExpr<Number> {
    using FunExpr<Number>::FunExpr;

    std::optional<Expression<Number>> subs(const std::string& name, const Expression<Number>& value) const override {
        auto expr_ = this->expr.subs_maybe(name, value);
        if (expr_) {
            return Expression<Number>(std::make_shared<Self<Number>>(expr_.value_or(this->expr)));
        } else {
            return {};
        }
    }

    bool operator==(const Expression<Number>& other) const override {
        FunExpr<Number>* v;
        return (v = dynamic_cast<Self<Number>*>(other.inner.get())) && v->expr == this->expr;
    }

    int precedence() const override {
        return 4;
    }
};

template<typename Number = DefaultNumber>
struct SinExpr : FunExprImpl<Number, SinExpr> {
    using FunExprImpl<Number, SinExpr>::FunExprImpl;

    Number eval() const override {
        return sin(this->expr.eval());
    }

    Expression<Number> diff(const std::string& name) const override {
        return cos(this->expr) * this->expr.diff(name);
    }

    std::string to_string() const override {
        return std::format("sin({})", this->expr.to_string());
    }
};

template<typename Number = DefaultNumber>
struct CosExpr : FunExprImpl<Number, CosExpr> {
    using FunExprImpl<Number, CosExpr>::FunExprImpl;

    Number eval() const override {
        return cos(this->expr.eval());
    }

    Expression<Number> diff(const std::string& name) const override {
        return -sin(this->expr) * this->expr.diff(name);
    }

    std::string to_string() const override {
        return std::format("cos({})", this->expr.to_string());
    }
};

template<typename Number = DefaultNumber>
struct LnExpr : FunExprImpl<Number, LnExpr> {
    using FunExprImpl<Number, LnExpr>::FunExprImpl;

    Number eval() const override {
        return log(this->expr.eval());
    }

    Expression<Number> diff(const std::string& name) const override {
        return this->expr.diff(name) / this->expr;
    }

    std::string to_string() const override {
        return std::format("ln({})", this->expr.to_string());
    }
};

template<typename Number = DefaultNumber>
struct ExpExpr : FunExprImpl<Number, ExpExpr> {
    using FunExprImpl<Number, ExpExpr>::FunExprImpl;

    Number eval() const override {
        return exp(this->expr.eval());
    }

    Expression<Number> diff(const std::string& name) const override {
        return exp(this->expr) * this->expr.diff(name);
    }

    std::string to_string() const override {
        return std::format("exp({})", this->expr.to_string());
    }
};

template<typename Number>
class Parser;

template<typename Number = DefaultNumber>
struct Expression {
    std::shared_ptr<Expr<Number>> inner;

    explicit Expression(std::shared_ptr<Expr<Number>> value) : inner(value) {};

    explicit Expression(const std::string& value) {
        inner = Parser<Number>(value).parse().inner;
    }

    static Expression<Number> var(const std::string& name) {
        return Expression(std::make_shared<VarExpr<Number>>(name));
    }

    Expression(std::type_identity_t<Number> value) : inner(std::make_shared<NumExpr<Number>>(value)) {}

    std::optional<Expression<Number>> subs_maybe(const std::string& name, const Expression<Number>& value) const {
        return inner->subs(name, value);
    }

    Expression<Number> subs(const std::string& name, const Expression<Number>& value) const {
        return inner->subs(name, value).value_or(*this);
    }

    Expression<Number> diff(const std::string& name) const {
        return inner->diff(name);
    }

    Number eval() const {
        return inner->eval();
    }

    std::string to_string() const {
        return inner->to_string();
    }

    int precedence() const {
        return inner->precedence();
    }

    friend bool operator==(const Expression<Number>& lhs, const Expression<Number>& rhs) {
        return *lhs.inner == rhs;
    }
    friend Expression<Number> operator+(Expression<Number> lhs, Expression<Number> rhs) {
        NumExpr<Number>* num;
        if ((num = dynamic_cast<NumExpr<Number>*>(lhs.inner.get())) && num->value == Number(0)) {
            return rhs;
        }
        if ((num = dynamic_cast<NumExpr<Number>*>(rhs.inner.get())) && num->value == Number(0)) {
            return lhs;
        }
        return Expression(std::make_shared<SumExpr<Number>>(lhs, rhs));
    }
    friend Expression<Number> operator-(Expression<Number> expr) {
        NumExpr<Number>* num;
        if ((num = dynamic_cast<NumExpr<Number>*>(expr.inner.get())) && num->value == Number(0)) {
            return expr;
        }
        return Expression(std::make_shared<NegExpr<Number>>(expr));
    }
    friend Expression<Number> operator-(Expression<Number> lhs, Expression<Number> rhs) {
        return lhs + (-rhs);
    }
    friend Expression<Number> operator*(Expression<Number> lhs, Expression<Number> rhs) {
        NumExpr<Number>* num;
        if ((num = dynamic_cast<NumExpr<Number>*>(lhs.inner.get()))) {
            if (num->value == Number(0)) return lhs;
            if (num->value == Number(1)) return rhs;
        }
        if ((num = dynamic_cast<NumExpr<Number>*>(rhs.inner.get()))) {
            if (num->value == Number(0)) return rhs;
            if (num->value == Number(1)) return lhs;
        }
        return Expression(std::make_shared<MulExpr<Number>>(lhs, rhs));
    }
    friend Expression<Number> operator/(Expression<Number> lhs, Expression<Number> rhs) {
        return Expression(std::make_shared<DivExpr<Number>>(lhs, rhs));
    }
    friend Expression<Number> pow(Expression<Number> base, Expression<Number> exponent) {
        NumExpr<Number>* num;
        if ((num = dynamic_cast<NumExpr<Number>*>(exponent.inner.get())) && num->value == Number(1)) {
            return base;
        }
        return Expression(std::make_shared<PowExpr<Number>>(base, exponent));
    }
    friend Expression<Number> operator^(Expression<Number> base, Expression<Number> exponent) {
        return pow(base, exponent);
    }
    friend Expression<Number> sin(Expression<Number> expr) {
        return Expression(std::make_shared<SinExpr<Number>>(expr));
    }
    friend Expression<Number> cos(Expression<Number> expr) {
        return Expression(std::make_shared<CosExpr<Number>>(expr));
    }
    friend Expression<Number> ln(Expression<Number> expr) {
        return Expression(std::make_shared<LnExpr<Number>>(expr));
    }
    friend Expression<Number> exp(Expression<Number> expr) {
        return Expression(std::make_shared<ExpExpr<Number>>(expr));
    }    
};

template<typename Number = DefaultNumber>
std::ostream& operator<<(std::ostream& os, const Expression<Number>& expr)
{
    os << expr.to_string();
    return os;
}

template<typename Number>
struct std::formatter<Expression<Number>> : std::formatter<std::string> {
    auto format(Expression<Number> expr, format_context& ctx) const {
        return formatter<string>::format(expr.to_string(), ctx);
    }
};

#include "parser.h"
