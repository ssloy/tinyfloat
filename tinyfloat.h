#include <iostream>
#include <cstdint>

struct TinyFloat {
    bool     negative = false;
    int16_t  exponent = 0;     // [-127 ... 128]
    uint32_t mantissa = 0;     // [2^23 ... 2^24)

    TinyFloat() = default;
    TinyFloat(bool negative, int16_t exponent, uint32_t mantissa);
    TinyFloat(int); // converting constructors
    TinyFloat(float);

    bool isnan() const { return exponent == 128 &&  mantissa; }
    bool isinf() const { return exponent == 128 && !mantissa; }
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isnormal() const { return exponent > -127; }
};

struct FixedPoint {
#include "digits.h"
    int number[nbits] = {};

    FixedPoint(int offset, uint32_t n);
    bool print(std::ostream& out, bool trailing=true, int carry=0, int digno=0) const;
};

std::ostream& operator<<(std::ostream& out, const TinyFloat& f);

bool operator==(const TinyFloat& lhs, const TinyFloat& rhs);
bool operator!=(const TinyFloat& lhs, const TinyFloat& rhs);
bool operator<=(const TinyFloat& lhs, const TinyFloat& rhs);
bool operator>=(const TinyFloat& lhs, const TinyFloat& rhs);
bool operator< (const TinyFloat& lhs, const TinyFloat& rhs);
bool operator> (const TinyFloat& lhs, const TinyFloat& rhs);

TinyFloat operator+(const TinyFloat &lhs, const TinyFloat &rhs);
TinyFloat operator-(const TinyFloat &lhs, const TinyFloat &rhs);
TinyFloat operator*(const TinyFloat &lhs, const TinyFloat &rhs);
TinyFloat operator/(const TinyFloat &lhs, const TinyFloat &rhs);
TinyFloat operator-(const TinyFloat &f);

TinyFloat fabs(const TinyFloat &f);
TinyFloat floor(const TinyFloat &f);
TinyFloat log(const TinyFloat &f);

