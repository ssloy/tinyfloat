#include <bit>
#include <limits>
#include <cassert>
#include "tinyfloat.h"
#include "printer.h"

TinyFloat::TinyFloat(bool neg, int16_t exp, uint32_t mant) : negative(neg), exponent(exp), mantissa(mant) {
    while (mantissa < (1<<23) && exponent > -126) { // TODO decide what to do if the normalization does not work
        mantissa = mantissa * 2;
        exponent = exponent - 1;
    }
    while (mantissa >= (1<<24) && exponent < 127) {
        mantissa = mantissa / 2;
        exponent = exponent + 1;
    }
}

TinyFloat::TinyFloat(int i) {
    negative = i<0;
    exponent = 23;
    mantissa = i<0? -i : i;
    if (!mantissa) { // zero
        negative = false;
        exponent = -126;
        return;
    }
    *this = { negative, exponent, mantissa }; // normalize
}

TinyFloat::TinyFloat(float f) { // nan/inf are correctly handled
    const uint32_t u = std::bit_cast<uint32_t>(f);
    uint32_t sign_bit     = (u >> 31) % 2;
    uint32_t raw_exponent = (u >> 23) % 256;
    uint32_t raw_mantissa =  u % (1<<23);

    negative = sign_bit;
    exponent = raw_exponent - 127;
    mantissa = raw_mantissa;

    if (exponent==-127) { // zero or subnormal
        exponent++;
    } else if (exponent<128) // normal, recover the hidden bit = 1
        mantissa = raw_mantissa + (1<<23);
}

TinyFloat::operator float() const { // nan/inf are correctly handled
    uint32_t sign_bit = negative;
    uint32_t raw_exponent = exponent+127;
    uint32_t raw_mantissa = mantissa % (1<<23); // clear the hidden bit
    if (exponent==-126 && mantissa==0)
        raw_exponent = 0; // zero
    return std::bit_cast<float>((sign_bit<<31) | (raw_exponent<<23) | raw_mantissa);
}

std::ostream& operator<<(std::ostream& out, const TinyFloat& f) {
    if (f.isnan()) {
        out << "nan";
    } else {
        if (f.negative) out << "-";
        if (f.isinf())  out << "inf";
        else out << Q128_149(126 + f.exponent, f.mantissa);
    }
    return out;
}

bool operator==(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.isnan() || rhs.isnan()) return false;  // NaNs are unordered
    if (lhs.mantissa == 0 && rhs.mantissa==0 && lhs.exponent==-126 && rhs.exponent==-126) return true; // +0 = -0
    return lhs.mantissa == rhs.mantissa && lhs.exponent == rhs.exponent && lhs.negative == rhs.negative;
}

bool operator!=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return !(lhs == rhs);
}

bool operator<(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.isnan() || rhs.isnan() || lhs==rhs) return false;
    if (lhs.negative != rhs.negative)      // positive > negative
        return lhs.negative;
    return lhs.negative !=                 // same sign and not equal
        ((lhs.exponent <  rhs.exponent) || // => check exponents and then mantissas
         (lhs.exponent == rhs.exponent && lhs.mantissa < rhs.mantissa));
}

bool operator>(const TinyFloat& lhs, const TinyFloat& rhs) {
    if (lhs.isnan() || rhs.isnan()) return false; // NaNs are unordered
    return !(lhs<rhs || lhs==rhs);
}

bool operator<=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return lhs<rhs || lhs==rhs;
}

bool operator>=(const TinyFloat& lhs, const TinyFloat& rhs) {
    return lhs>rhs || lhs==rhs;
}

TinyFloat operator+(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat a = lhs;
    TinyFloat b = rhs;
    if (a.isnan() || b.isnan())
        return TinyFloat::nan();

    if (a.isinf() && b.isinf()) {
        if (a.negative == b.negative) return a; // same sign infinity
        return TinyFloat::nan();                // inf + -inf = NaN
    }
    if (a.isinf()) return a;
    if (b.isinf()) return b;

    if (a.mantissa == 0 && b.mantissa==0 && a.exponent==-126 && b.exponent==-126) // handle zeros
        return {(a.negative == b.negative) ? a.negative : false, -126, 0};        // if signs differ, result is +0

    if (a.exponent < b.exponent)
        std::swap(a, b);

    while (a.exponent > b.exponent) {
        b.mantissa /=  2;
        b.exponent++;
    }

    bool negative;
    uint32_t mantissa;

    negative = a.negative;
    if (a.negative==b.negative) {
        mantissa = a.mantissa + b.mantissa;
    } else {
        if (a.mantissa>=b.mantissa) {
            mantissa = a.mantissa - b.mantissa;
        } else {
            negative = !a.negative;
            mantissa = b.mantissa - a.mantissa;
        }
    }
    // When the sum of two operands with opposite signs (or the difference of two operands with like signs) is exactly zero, the sign of that sum (or difference) shall be +0
    if (!mantissa)
        negative = false;
    return {negative, a.exponent, mantissa};
}
/*

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
*/

#if 0

TinyFloat fabs(const TinyFloat &f) {
    return {false, f.exponent, f.mantissa};
}

TinyFloat floor(const TinyFloat &f) {
    if (f.exponent < 0)
        return (f < 0) ? -1 : 0;  // If |x| < 1, floor is -1 or 0

    uint32_t mask = (1 << (23 - f.exponent)) - 1; // mask to clear fraction bits
    if (!(f.mantissa & mask)) return f; // Already an integer

    uint32_t mantissa = f.mantissa & ~mask; // clear fractional part

    if (f < 0) return TinyFloat{true, f.exponent, mantissa} - 1; // floor correction for negatives
    return {f.negative, f.exponent, mantissa};
}

/*
#define LN2 0.69314718f  // Natural log of 2
// Polynomial approximation for log(1 + x) in [1, 2)
static inline TinyFloat log1p_approx(const TinyFloat &m) {
    // Remez polynomial approximation of log(1 + x) for x in [0, 1)
    TinyFloat m2 = m * m;
    return m * (0.9999964239f + m * (-0.4998741238f + m * (0.3317990251f + m * (-0.2407338085f + m * 0.1676540711f))));
}
*/

#define LN2 0.69314718055994530942f  // Natural log(2)

// 7th-degree minimax polynomial for log(1 + x) in [1, 2)
static inline TinyFloat log1p_approx(const TinyFloat m) {
    TinyFloat m2 = m * m;
    TinyFloat m4 = m2 * m2;
    return m * (0.999999925f + m * (-0.499999994f + m * (0.333333313f + m * (-0.250000000f + m * (0.199999988f + m * (-0.166666656f + m * 0.142857134f))))));
}


TinyFloat log(const TinyFloat &f) {
//TODO NAN     if (f <= 0)
/*
    if (x < 0.0f) return 0.0f / 0.0f;   // NaN for log(negative)
    if (x == 0.0f) return -1.0f / 0.0f; // -Inf for log(0)
    if (x == 1.0f) return 0.0f;         // log(1) = 0
*/

    TinyFloat a {false, f.exponent, f.mantissa & ~(1<<23)};

    return TinyFloat(f.exponent) * LN2 + log1p_approx(a);

}
#endif

