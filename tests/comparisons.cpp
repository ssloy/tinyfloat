#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("comparisons") {
    float vals[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        123.f,
        -123.f,
        123.4f,
        -123.4f,
        std::numeric_limits<float>::denorm_min(), // smallest positive subnormal
        -std::numeric_limits<float>::denorm_min(),
        std::numeric_limits<float>::min(),   // smallest positive normal
        -std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN()
    };

    for (float a : vals) {
        for (float b : vals) {
            TinyFloat sa(a);
            TinyFloat sb(b);

            bool eq   = (a == b);
            bool ne   = (a != b);
            bool lt   = (a <  b);
            bool le   = (a <= b);
            bool gt   = (a >  b);
            bool ge   = (a >= b);

            bool s_eq = (sa == sb);
            bool s_ne = (sa != sb);
            bool s_lt = (sa < sb);
            bool s_le = (sa <= sb);
            bool s_gt = (sa > sb);
            bool s_ge = (sa >= sb);

            if (std::isnan(a) || std::isnan(b)) { // for NaNs: only "!=" is true, everything else must be false
                CHECK( (!s_eq && s_ne && !s_lt && !s_le && !s_gt && !s_ge) );
            } else {
                CHECK(eq == s_eq);
                CHECK(ne == s_ne);
                CHECK(lt == s_lt);
                CHECK(le == s_le);
                CHECK(gt == s_gt);
                CHECK(ge == s_ge);
            }

        }
    }
}

