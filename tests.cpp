#undef NDEBUG
#include <cassert>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
#include <limits>
#include "tinyfloat.h"

void test_float_roundtrip() {
    // finite values
    float values[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        0.5f,
        123.456f,
        std::numeric_limits<float>::min(),
        std::numeric_limits<float>::max()};

    for (float f : values) {
        TinyFloat forth(f);
        float back = forth;
        assert((std::isnan(f) && std::isnan(back)) || f == back);
    }

    // infinities
    assert(std::isinf(TinyFloat(std::numeric_limits<float>::infinity())));
    assert(std::isinf(TinyFloat(-std::numeric_limits<float>::infinity())) && std::signbit(TinyFloat(-std::numeric_limits<float>::infinity())));

    // NaN
    assert(std::isnan(TinyFloat(std::numeric_limits<float>::quiet_NaN())));
}

void test_int_roundtrip() {
    int values[] = {0, 1, -1, 42, -999999, 1<<23, -(1<<23)};

    for (int x : values)
        assert(x == int(TinyFloat(x)));
}

void test_printing() {
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
        assert((std::isnan(v) && std::isnan(f)) ||  v == f);
    }

    std::ostringstream o;
    for (float f : values)
        o << TinyFloat(f) << "\n";
    assert(o.str() == expected);
}

void test_comparisons() {
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
                assert(!s_eq && s_ne && !s_lt && !s_le && !s_gt && !s_ge);
            } else {
                assert(eq == s_eq);
                assert(ne == s_ne);
                assert(lt == s_lt);
                assert(le == s_le);
                assert(gt == s_gt);
                assert(ge == s_ge);
            }

        }
    }
}

void test_sum() {
    float values[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        0.5f,
        -0.5f,
        2.0f,
        -2.0f,
//      std::ldexp(1.0f, -24),       // 2^-24 (rounding boundary)
//      std::ldexp(1.0f, -25),       // 2^-25 (sticky case)
//      std::nextafterf(1.0f, 0.0f), // just below 1.0
//      std::nextafterf(1.0f, 2.0f), // just above 1.0
//      std::ldexp(1.0f, -126),      // smallest normal
//      std::ldexp(1.0f, -149),      // smallest subnormal
        1e20f,                       // large finite
        -1e20f,
//      std::numeric_limits<float>::max(),
//      -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
        std::numeric_limits<float>::quiet_NaN()
    };

    for (float a : values) {
        for (float b : values) {
            std::cerr << a << " + " << b << std::endl;
            TinyFloat sa(a);
            TinyFloat sb(b);
            float ref = a + b;
            float got = sa + sb;

            if (std::isnan(ref)) { // both should be NaN
                assert(std::isnan(got));
            } else if (ref == 0.0f && got == 0.0f) { // check signed zero
            std::cerr << ref << " " << got << "\n";
                assert(std::signbit(ref) == std::signbit(got));
            } else { // compare exact bits
                if (ref!=got)
                std::cerr << a << " + " << b << " = " << ref << " != " << got << std::endl;
                assert(ref == got);
            }
        }
    }
}


int main() {
    test_float_roundtrip();
    test_int_roundtrip();
    test_printing();
    test_comparisons();
    test_sum();

    return 0;
}

