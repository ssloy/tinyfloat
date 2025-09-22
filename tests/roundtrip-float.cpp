#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("float cast roundtrip") {
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

    for (float f : values) {
        TinyFloat forth(f);
        float back = forth;
        if (std::isinf(f)) {
            CHECK( std::signbit(f) == std::signbit(back) );
        } else {
//          std::cerr << f << " " << forth << " " << back << std::endl;
            CHECK( ((std::isnan(f) && std::isnan(back)) || f == back) );
        }
    }

//  // infinities
//  CHECK(std::isinf(TinyFloat(std::numeric_limits<float>::infinity())));
//  CHECK( (std::isinf(TinyFloat(-std::numeric_limits<float>::infinity())) && std::signbit(TinyFloat(-std::numeric_limits<float>::infinity()))) );

}

