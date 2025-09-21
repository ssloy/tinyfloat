#include <iostream>
#include <cstdint>

struct TinyFloat {
    bool     negative = false;
    int16_t  exponent = -126;  // [-126 ... 128], corrected exponent
    uint32_t mantissa = 0;     // [0 ... 2^24), so mantissa/2^23 is in [0, 2) range

    TinyFloat(bool negative, int16_t exponent, uint32_t mantissa);
    TinyFloat() = default;
    TinyFloat(const TinyFloat&) = default;
    TinyFloat& operator=(const TinyFloat&) = default;

    TinyFloat(int); // casts
    TinyFloat(float);
    operator float() const;

    bool isnan() const { return exponent == 128 &&  mantissa; }
    bool isinf() const { return exponent == 128 && !mantissa; }
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isnormal() const { return isfinite() && mantissa >= (1<<23); }

    static TinyFloat nan() { return {0, 128, (1<<24)-1}; }
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


/*
TinyFloat fabs(const TinyFloat &f);
TinyFloat floor(const TinyFloat &f);
TinyFloat log(const TinyFloat &f);
*/

