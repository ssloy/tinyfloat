#include <iostream>
#include <cstdint>

struct TinyFloat {
    bool negative = false;
    int8_t exponent = 0;    // [-127 ... 127]
    uint32_t mantissa = 0;  // [2^23 ... 2^24)
};

struct FixedPoint {
#include "digits.h"
    int number[nbits] = {};

    FixedPoint(int offset, uint32_t n);
    bool print(std::ostream& out, bool trailing=true, int carry=0, int digno=0) const;
};

std::ostream& operator<<(std::ostream& out, const TinyFloat& f);

