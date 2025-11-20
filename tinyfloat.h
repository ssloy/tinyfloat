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

    TinyFloat(int);
    TinyFloat(float);
    operator float() const;

    bool isnan() const { return exponent == 128 &&  mantissa; }
    bool isinf() const { return exponent == 128 && !mantissa; }
    bool isfinite() const { return !isnan() && !isinf(); }
    bool isnormal() const { return isfinite() && mantissa >= (1u<<23); }

    static TinyFloat  nan(uint32_t payload = (1u<<24)-1) { return {false,  128, payload}; }
    static TinyFloat  inf(bool negative = false) { return {negative,  128, 0}; }
    static TinyFloat zero(bool negative = false) { return {negative, -126, 0}; }
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

