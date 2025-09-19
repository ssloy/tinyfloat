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

    std::ostringstream s;
    s << TinyFloat(0.0f) << "\n";
    s << TinyFloat(1.5f) << "\n";
    s << TinyFloat(std::numeric_limits<float>::denorm_min()) << "\n";
    s << TinyFloat(-std::numeric_limits<float>::min()) << "\n";
    s << TinyFloat(std::numeric_limits<float>::max()) << "\n";
    s << TinyFloat(std::numeric_limits<float>::infinity()) << "\n";
    s << TinyFloat(-std::numeric_limits<float>::infinity()) << "\n";
    s << TinyFloat(std::numeric_limits<float>::quiet_NaN()) << "\n";
    assert(s.str() == expected);
}

void check_compare(float a, float b) {
    TinyFloat sa(a);
    TinyFloat sb(b);

    // Native float comparisons
    bool eq   = (a == b);
    bool ne   = (a != b);
    bool lt   = (a <  b);
    bool le   = (a <= b);
    bool gt   = (a >  b);
    bool ge   = (a >= b);

    // TinyFloat comparisons
    bool s_eq = (sa == sb);
    bool s_ne = (sa != sb);
    bool s_lt = (sa < sb);
    bool s_le = (sa <= sb);
    bool s_gt = (sa > sb);
    bool s_ge = (sa >= sb);

    // For NaNs: only "!=" is true, everything else must be false
    if (std::isnan(a) || std::isnan(b)) {
        assert(!s_eq && s_ne && !s_lt && !s_le && !s_gt && !s_ge);
    } else {
        assert(eq == s_eq);
        assert(ne == s_ne);
        assert(lt == s_lt);
        assert(le == s_le);
        std::cerr << a << " " << sa << " " << b << " " << sb << std::endl;
        assert(gt == s_gt);
        assert(ge == s_ge);
    }
}

void test_comparisons() {
    float vals[] = {
        0.0f,
        -0.0f,
        1.0f,
        -1.0f,
        123.5f,
        -123.5f,
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

    // Compare every pair
    for (float a : vals) {
        for (float b : vals) {
            check_compare(a, b);
        }
    }

    std::cout << "All comparison tests passed!\n";
}

int main() {
    test_float_roundtrip();
    test_int_roundtrip();
    test_printing();
    test_comparisons();
    return 0;
}

