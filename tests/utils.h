#ifndef HEADER_UTILS
#define HEADER_UTILS

#include<source_location>
#include<atomic>
#include<sstream>
#include<iostream>
#include<complex>

static std::atomic<unsigned int> ALL_ASSERTIONS_NUMBER = 0;
static std::atomic<unsigned int> FAIL_ASSERTIONS_NUMBER = 0;

inline void assert(bool value, std::string message = "", std::source_location loc = std::source_location::current()) {
    ALL_ASSERTIONS_NUMBER++;
    if (!value) {
        FAIL_ASSERTIONS_NUMBER++;
    }
    std::string status = value ? "\033[30;42m<OK>\033[0m" : "\033[30;41m<FAIL>\033[0m";
    std::cout << "Test " << ALL_ASSERTIONS_NUMBER << " `" << loc.function_name() << "`: " << status;
    std::cout << " [" << loc.file_name() << ":" << loc.line() << "]";
    if (!value && !message.empty()) {
        std::cout << "\n    (" << message << ")";
    }
    std::cout << std::endl;
}

template<typename X, typename Y> requires requires(X x, Y y) { x == y; }
inline void assert_eq(X x, Y y, std::source_location loc = std::source_location::current()) {
    bool value = x == y;
    std::stringstream message;
    if constexpr (requires { message << x; message << y; }) {
        if (!value) {
            message << x;
            message << " != ";
            message << y;
        }
    }
    assert(value, message.str(), loc);
}

template <typename ExceptionType, typename Func>
inline void assert_throws(Func&& func, std::source_location loc = std::source_location::current()) {
    bool value = false;
    std::stringstream message;
    try {
        func();
        message << "Expected exception, but none was thrown";
    } catch (const ExceptionType&) {
        value = true;
    } catch (const std::exception& e) {
        message << "Unexpected exception caught: " << e.what();
    } catch (...) {
        message << "Unexpected exception caught";
    }
    assert(value, message.str(), loc);
}

using std::abs;

template<typename X, typename Y> requires requires(X x, Y y) { abs(x - y); }
inline void assert_close(X x, Y y, double epsilon = 1e-6, std::source_location loc = std::source_location::current()) {
    auto value = abs(x - y) <= epsilon;
    std::stringstream message;
    if (!value) {
        message << x << " not within " << epsilon << " of " << y;
    }
    assert(value, message.str(), loc);
}

inline void summary() {
    if (FAIL_ASSERTIONS_NUMBER == 0) {
        std::cout << "\033[30;42mSUCCESS\033[0m: No failed assertions.\n";
    } else {
        std::cout << "\033[30;41mFAIL\033[0m: " << FAIL_ASSERTIONS_NUMBER << " assertions failed.\n";
    }
    std::cout << (ALL_ASSERTIONS_NUMBER - FAIL_ASSERTIONS_NUMBER) << "/" << ALL_ASSERTIONS_NUMBER;
    std::cout << " tests OK." << std::endl;
}

#endif
