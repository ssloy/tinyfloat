#include "tinyfloat.h"

FixedPoint::FixedPoint(int offset, uint32_t n) {
    int cnt = 0;
    while (n > 0) {
        number[offset + cnt++] = n % 2;
        n = n / 2;
    }
}

bool FixedPoint::print(std::ostream& out, bool trailing, int carry, int digno) const {
    int sum = 0;
    for (int i=0; i<nbits; i++)
        sum += number[i] * digits[digno][i];
    int digit = (sum + carry) % 10;
    bool leading = !digit && digno > dotpos + 1;
    trailing    &= !digit && digno < dotpos;
    if (digno < ndigits-1)
        leading &= print(out, trailing, (sum + carry)/10, digno + 1);
    if (digno == dotpos) out << ".";
    if (!leading && !trailing) out << digit;
    return leading;
}

std::ostream& operator<<(std::ostream& out, const TinyFloat& f) { // TODO inf/nan
    if (f.negative) out << "-";
    FixedPoint fix(126 + f.exponent, f.mantissa);
    fix.print(out);
    return out;
}

bool operator==(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.mantissa == 0 && rhs.mantissa==0 && lhs.exponent==0 && rhs.exponent==0) return true; // +0 = -0
    if (lhs.isnan() || rhs.isnan()) return false;
    return lhs.mantissa == rhs.mantissa && lhs.exponent == rhs.exponent && lhs.negative == rhs.negative;
}

bool operator!=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return !(lhs == rhs);
}

bool operator<(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.isnan() || rhs.isnan()) return false;
    if (lhs.mantissa == 0 && rhs.mantissa==0 && lhs.exponent==0 && rhs.exponent==0) return false; // +0 = -0
    if (lhs.exponent < rhs.exponent || lhs.mantissa < rhs.mantissa)
        return !rhs.negative;
    return lhs.negative && !rhs.negative;
}

bool operator>(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.isnan() || rhs.isnan()) return false;
    if (lhs.mantissa == 0 && rhs.mantissa==0 && lhs.exponent==0 && rhs.exponent==0) return false; // +0 = -0
    if (lhs.exponent > rhs.exponent || lhs.mantissa > rhs.mantissa)
        return !lhs.negative;
    return !lhs.negative && rhs.negative;
}

bool operator<=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return lhs<rhs || lhs==rhs;
}

bool operator>=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return lhs>rhs || lhs==rhs;
}

