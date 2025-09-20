#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("printing") {
    std::string expected("0.0\n\
-0.0\n\
1.5\n\
0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125\n\
-0.000000000000000000000000000000000000011754943508222875079687365372222456778186655567720875215087517062784172594547271728515625\n\
340282346638528859811704183484516925440.0\n\
inf\n\
-inf\n\
nan\n\
");

    float values[] = {
        0.0f,
        -0.0f,
        1.5f,
        std::numeric_limits<float>::denorm_min(),
        -std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN()
    };

    for (float f : values) {
        std::ostringstream o;
        o << TinyFloat(f);
        std::istringstream i(o.str());
        float v = std::stof(o.str());
        CHECK( ((std::isnan(v) && std::isnan(f)) ||  v == f) );
    }

    std::ostringstream o;
    for (float f : values)
        o << TinyFloat(f) << "\n";
    CHECK(o.str() == expected);
}

