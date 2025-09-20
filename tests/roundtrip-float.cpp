#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("float cast roundtrip") {
    // finite values
    float values[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        0.5f,
        123.456f,
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max()
    };

    for (float f : values) {
        TinyFloat forth(f);
        float back = forth;
        assert((std::isnan(f) && std::isnan(back)) || f == back);
    }

    // infinities
    CHECK(std::isinf(TinyFloat(std::numeric_limits<float>::infinity())));
    CHECK( (std::isinf(TinyFloat(-std::numeric_limits<float>::infinity())) && std::signbit(TinyFloat(-std::numeric_limits<float>::infinity()))) );

    // NaN
    CHECK(std::isnan(TinyFloat(std::numeric_limits<float>::quiet_NaN())));
}

