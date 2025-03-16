#include <cassert>
#include "tinyfloat.h"

TinyFloat::TinyFloat(bool n, int16_t e, uint32_t m) : negative(n), exponent(e), mantissa(m) {
    if (!mantissa) {
        negative = false;
        exponent = 0;
        return;
    }

    while (mantissa < (1<<23) && exponent > -127) {
        mantissa = mantissa * 2;
        exponent = exponent - 1;
    }
    while (mantissa > (1<<24)  && exponent < 128) { // TODO decide what to do if the normalization does not work
        mantissa = mantissa / 2;
        exponent = exponent + 1;
    }
}

TinyFloat::TinyFloat(int i) {
    *this = TinyFloat(i<0, 23, i<0?-i:i);
}

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

TinyFloat operator+(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat a = lhs; // TODO edge cases
    TinyFloat b = rhs;

    bool negative;
    uint32_t mantissa;

    while (a.exponent > b.exponent) {
        b.mantissa /=  2;
        b.exponent++;
    }
    while (a.exponent < b.exponent) {
        a.mantissa /=  2;
        a.exponent++;
    }

    negative = a.negative;
    if (a.negative==b.negative) {
        mantissa = a.mantissa + b.mantissa;
    } else {
        mantissa = a.mantissa - b.mantissa;
        if (mantissa<0) {
            negative = !a.negative;
            mantissa = -mantissa;
        }
    }
    return {negative, a.exponent, mantissa};
}

TinyFloat operator-(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat f(!rhs.negative, rhs.exponent, rhs.mantissa);
    return lhs + f;
}

TinyFloat operator*(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat a = lhs; // TODO edge cases
    TinyFloat b = rhs;

    uint32_t a_hi = a.mantissa / 4096; // multiply 2 24-bit mantissas
    uint32_t a_lo = a.mantissa % 4096; // into two 24-bit halves mantissa_high, mantissa_low
    uint32_t b_hi = b.mantissa / 4096;
    uint32_t b_lo = b.mantissa % 4096;
    uint32_t hihi = a_hi * b_hi;
    uint32_t hilo = a_hi * b_lo;
    uint32_t lohi = a_lo * b_hi;
    uint32_t lolo = a_lo * b_lo;
    uint32_t mantissa_low = lolo + (hilo%4096 + lohi%4096)*4096;
    uint32_t mantissa_high = hihi + hilo/4096 + lohi/4096 + mantissa_low/16777216;
    mantissa_low = mantissa_low % 16777216;

    return {!a.negative != !b.negative, static_cast<int16_t>(a.exponent + b.exponent + 1), mantissa_high};
}

TinyFloat operator/(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat a = lhs; // TODO edge cases
    TinyFloat b = rhs;

    if (!a.mantissa) {
        return 0;
    }

    uint32_t mantissa  = a.mantissa / b.mantissa;
    uint32_t remainder = a.mantissa % b.mantissa;
    int16_t  exponent  = a.exponent - b.exponent - 1 + 24;

    if (!mantissa) { // TODO: meh
        mantissa = 1;
        exponent--;
        remainder *= 2;
        if (remainder >= b.mantissa) {
            remainder -= b.mantissa;
        }
    }
    while (mantissa < 8388608) {
        mantissa = mantissa * 2;
        remainder = remainder * 2;
        exponent = exponent - 1;
        if (remainder >= b.mantissa) {
            mantissa++;
            remainder -= b.mantissa;
        }
    }

    return {!a.negative != !b.negative, exponent, mantissa};
}

TinyFloat operator-(const TinyFloat &f) {
    return {!f.negative, f.exponent, f.mantissa};
}

TinyFloat fabs(const TinyFloat &f) {
    return {false, f.exponent, f.mantissa};
}

