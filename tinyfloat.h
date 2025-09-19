#include <iostream>
#include <cstdint>

struct TinyFloat {
    bool     negative = false;
    int16_t  exponent = 0;     // [-126 ... 128], corrected exponent
    uint32_t mantissa = 0;     // [0 ... 2^24), so mantissa/2^23 is in [0, 2) range

    TinyFloat() = default;
    TinyFloat(bool negative, int16_t exponent, uint32_t mantissa);
    TinyFloat(int); // converting constructors
    TinyFloat(float);

    operator float() const;

    bool isnan() const { return exponent == 128 &&  mantissa; }
    bool isinf() const { return exponent == 128 && !mantissa; }
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isnormal() const { return isfinite() && mantissa >= (1<<23); }
};

struct Q128_149 {
#include "digits.h"
    int number[nbits] = {};

    Q128_149(int offset, uint32_t n);
    bool print(std::ostream& out, bool trailing=true, int carry=0, int digno=0) const;
};

std::ostream& operator<<(std::ostream& out, const TinyFloat& f);

/*

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
*/

/*
TinyFloat fabs(const TinyFloat &f);
TinyFloat floor(const TinyFloat &f);
TinyFloat log(const TinyFloat &f);
*/

