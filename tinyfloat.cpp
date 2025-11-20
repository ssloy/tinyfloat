#include <bit>
#include <cassert>
#include "tinyfloat.h"
#include "printer.h"

TinyFloat normalized(TinyFloat f) { // TODO remove this
    while (f.mantissa < (1<<23) && f.exponent > -126) {
        f.mantissa = f.mantissa * 2;
        f.exponent = f.exponent - 1;
    }
    while (f.mantissa >= (1<<24) || f.exponent < -126) {
        if (f.exponent >= 127) {
            f.mantissa = 0;
            f.exponent = 128;
            break;
        }
        f.mantissa = f.mantissa / 2;
        f.exponent = f.exponent + 1;
    }

    return f;
}

TinyFloat::TinyFloat(bool neg, int16_t exp, uint32_t mant) : negative(neg), exponent(exp), mantissa(mant) {
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
    *this = normalized({ negative, exponent, mantissa });
}

TinyFloat::TinyFloat(float f) { // nan/inf are correctly handled
    const uint32_t u = std::bit_cast<uint32_t>(f);
    uint32_t sign_bit     = (u >> 31) % 2;
    uint32_t raw_exponent = (u >> 23) % 256;
    uint32_t raw_mantissa =  u % (1<<23);

    negative = sign_bit;
    exponent = raw_exponent - 127;
    mantissa = raw_mantissa;

    if (exponent==-127) // zero or subnormal
        exponent++;
    else if (exponent<128) // normal, recover the hidden bit = 1
        mantissa = raw_mantissa + (1<<23);
}

TinyFloat::operator float() const { // nan/inf are correctly handled
    uint32_t sign_bit = negative;
    uint32_t raw_exponent = exponent+127;
    uint32_t raw_mantissa = mantissa % (1<<23); // clear the hidden bit
    if (exponent==-126 && mantissa<(1<<23))
        raw_exponent = 0; // zero or subnormal
    return std::bit_cast<float>((sign_bit<<31) + (raw_exponent<<23) + raw_mantissa);
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
    if (lhs.isfinite() && rhs.isfinite() && !lhs.mantissa && !rhs.mantissa) return true; // +0 = -0
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
        return TinyFloat::nan();                // inf + -inf = nan
    }
    if (a.isinf()) return a;
    if (b.isinf()) return b;

    if (!a.mantissa && !b.mantissa)                       // handle zeros
        return TinyFloat::zero(a.negative && b.negative); // if signs differ, result is +0

    if (a.exponent < b.exponent)
        std::swap(a, b);

    a.mantissa *= 8;                                  // reserve place for GRS bits
    b.mantissa *= 8;

    while (a.exponent > b.exponent) {                 // align exponents
        b.mantissa = (b.mantissa/2) | (b.mantissa%2); // LSB is sticky
        b.exponent++;
    }

    TinyFloat sum = { a.mantissa >= b.mantissa ? a.negative : b.negative, a.exponent, 0 };

    if (a.negative == b.negative)
        sum.mantissa = a.mantissa + b.mantissa;
    else
        if (a.mantissa >= b.mantissa)
            sum.mantissa = a.mantissa - b.mantissa;
        else
            sum.mantissa = b.mantissa - a.mantissa;

    while (sum.mantissa < (1u<<(23+3)) && sum.exponent > -126) { // normalize the result
        sum.mantissa *= 2;
        sum.exponent--;
    }

    while (sum.mantissa >= (1u<<(24+3))) {                  // can't be more than one iteration
        sum.mantissa = (sum.mantissa/2) | (sum.mantissa%2); // do not forget the sticky bit
        sum.exponent++;
    }

    uint32_t g = (sum.mantissa / 4) % 2;       // guard bit
    uint32_t r = (sum.mantissa / 2) % 2;       // round bit
    uint32_t s =  sum.mantissa % 2;            // sticky bit
    sum.mantissa /= 8;

    if (g && (r || s || (sum.mantissa % 2))) { // round-to-nearest, even-on-ties
        sum.mantissa++;
        if (sum.mantissa == (1u<<24)) {        // renormalize if necessary
            sum.mantissa /= 2;
            sum.exponent++;
        }
    }

    if (sum.exponent >= 128)               // handle overflow
        return TinyFloat::inf(sum.negative);

    if (!sum.mantissa)                     // When the sum of two operands with opposite signs (or the difference of two operands with like signs)
        return TinyFloat::zero();          // is exactly zero, the sign of that sum (or difference) shall be +0
    return sum;
}

TinyFloat operator-(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat f(!rhs.negative, rhs.exponent, rhs.mantissa);
    return lhs + f;
}

TinyFloat operator*(const TinyFloat &lhs, const TinyFloat &rhs) {
    TinyFloat a = lhs;
    TinyFloat b = rhs;
    if (a.isnan() || b.isnan())
        return TinyFloat::nan();
    if (a.isinf() || b.isinf()) {
        if ((a.isfinite() && !a.mantissa) || (b.isfinite() && !b.mantissa)) // inf * 0 = nan
            return TinyFloat::nan();
        return TinyFloat::inf(a.negative != b.negative);
    }
    if (!a.mantissa || !b.mantissa)
        return TinyFloat::zero(a.negative != b.negative);

    int16_t exponent = a.exponent + b.exponent + 1; // +1 comes from the separation of a.mantissa * b.mantissa into two 24-bit variables
    bool negative = a.negative != b.negative;

    uint32_t a_hi = a.mantissa / (1u<<12); // multiply 2 24-bit mantissas
    uint32_t a_lo = a.mantissa % (1u<<12); // into two 24-bit halves mantissa, mantissa_low
    uint32_t b_hi = b.mantissa / (1u<<12);
    uint32_t b_lo = b.mantissa % (1u<<12);
    uint32_t hihi = a_hi * b_hi;
    uint32_t hilo = a_hi * b_lo;
    uint32_t lohi = a_lo * b_hi;
    uint32_t lolo = a_lo * b_lo;
    uint32_t mantissa_low = lolo + (hilo % (1u<<12) + lohi % (1u<<12)) * (1u<<12);
    uint32_t mantissa = hihi +  hilo / (1u<<12) + lohi / (1u<<12) + mantissa_low/(1u<<24);
    mantissa_low = mantissa_low % (1u<<24);

    while (mantissa < (1u<<23) && exponent > -126) { // normalize the result
        mantissa = mantissa * 2 + mantissa_low / (1u<<23);
        mantissa_low = (mantissa_low * 2) % (1u<<24);
        exponent--;
    }

    while (exponent < -126) {
        mantissa_low = ((mantissa_low + (mantissa % 2) * (1u<<24))/2) | (mantissa_low % 2); // LSB is sticky
        mantissa /= 2;
        exponent++;
    }

    if (mantissa_low / (1u<<23) && (mantissa_low % (1u<<23) || mantissa % 2)) {
        mantissa++;
        if (mantissa == (1u<<24)) {    // renormalize if necessary
            mantissa /= 2;
            exponent++;
        }
    }

    if (exponent >= 128)               // handle overflow
        return TinyFloat::inf(negative);

    return { negative, exponent, mantissa };
}

TinyFloat operator/(const TinyFloat &a, const TinyFloat &b) {
    if (a.isnan() || b.isnan() || (a.isinf() && b.isinf()) || (!a.mantissa && !b.mantissa))
        return TinyFloat::nan();

    bool negative = a.negative != b.negative;
    if (a.isinf() || !b.mantissa)
        return TinyFloat::inf(negative);

    if (!a.mantissa || b.isinf())
        return TinyFloat::zero(negative);

    assert(a.isfinite() && b.isfinite() && a.mantissa && b.mantissa);

    uint32_t mantissa  = a.mantissa / b.mantissa;
    uint32_t remainder = a.mantissa % b.mantissa;
    int16_t  exponent  = a.exponent - b.exponent + 23;

    while (mantissa < (1u<<23) && exponent > -126) { // normalize the result
        remainder *= 2;
        mantissa = mantissa * 2 + remainder / b.mantissa;
        remainder = remainder % b.mantissa;
        exponent--;
    }

    while (exponent < -126) {
        remainder = (remainder + (mantissa % 2)*b.mantissa)/2 | (remainder % 2); // LSB is sticky
        mantissa /= 2;
        exponent++;
    }

    if (remainder*2 > b.mantissa || (remainder*2==b.mantissa && mantissa%2)) {
        mantissa++;
        if (mantissa == (1u<<24)) {    // renormalize if necessary
            mantissa /= 2;
            exponent++;
        }
    }

    if (exponent >= 128)               // handle overflow
        return TinyFloat::inf(negative);

    return { negative, exponent, mantissa };
}

TinyFloat operator-(const TinyFloat &f) {
    if (f.isnan()) return f;
    return { !f.negative, f.exponent, f.mantissa };
}


