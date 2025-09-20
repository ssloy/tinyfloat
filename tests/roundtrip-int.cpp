#define _USE_MATH_DEFINES
#include <cmath>
#include <limits>
#include "tinyfloat.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEST_CASE("int cast roundtrip") {
    int values[] = {0, 1, -1, 42, -999999, 1<<23, -(1<<23)};
    for (int x : values)
        CHECK(x == int(TinyFloat(x)));
}

