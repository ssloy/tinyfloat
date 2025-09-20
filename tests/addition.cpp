#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("addition") {
    float values[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        0.5f,
        -0.5f,
        2.0f,
        -2.0f,
        std::ldexp(1.0f, -24),       // 2^-24 (rounding boundary)
        std::ldexp(1.0f, -25),       // 2^-25 (sticky case)
        std::nextafterf(1.0f, 0.0f), // just below 1.0
        std::nextafterf(1.0f, 2.0f), // just above 1.0
        std::ldexp(1.0f, -126),      // smallest normal
        std::ldexp(1.0f, -149),      // smallest subnormal
        1e20f,                       // large finite
        -1e20f,
        std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN()
    };

    for (float a : values) {
        for (float b : values) {
            TinyFloat sa(a);
            TinyFloat sb(b);
            float ref = a + b;
            float got = sa + sb;
            if (std::isnan(ref)) { // both should be NaN
                CHECK(std::isnan(got));
            } else if (ref == 0.0f && got == 0.0f) { // check signed zero
                CHECK(std::signbit(ref) == std::signbit(got));
            } else { // compare exact bits
                CHECK(ref == got);
            }
        }
    }
}

