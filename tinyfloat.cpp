#include <bit>
#include <cassert>
#include "tinyfloat.h"
#include "printer.h"

TinyFloat normalized(TinyFloat f) {
    while (f.mantissa < (1<<23) && f.exponent > -126) { // TODO decide what to do if the normalization does not work
        f.mantissa = f.mantissa * 2;
        f.exponent = f.exponent - 1;
    }
    while (f.mantissa >= (1<<24) || f.exponent < -126) { // TODO smth in these lines above
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

bool operator==(const TinyFloat& lhs, const TinyFloat& rhs) { // TODO if mantissa is zero, should I care to check the exponents?
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

    if (a.mantissa == 0 && b.mantissa==0/* && a.exponent==-126 && b.exponent==-126*/) // handle zeros
        return {(a.negative == b.negative) ? a.negative : false, -126, 0};        // if signs differ, result is +0

    if (a.exponent < b.exponent)
        std::swap(a, b);

    a.exponent -= 3;
    b.exponent -= 3;
    a.mantissa *= 8;
    b.mantissa *= 8;

    while (a.exponent > b.exponent) {
        b.mantissa = (b.mantissa/2) | (b.mantissa%2);
//      b.mantissa /= 2;
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

    while (mantissa < (1<<(23+3)) && a.exponent > -126-3) { // TODO decide what to do if the normalization does not work
        mantissa = mantissa * 2;
        a.exponent = a.exponent - 1;
        // TODO underflow?
    }

    while (mantissa >= (1<<(24+3)) ) {
        mantissa = (mantissa / 2) | (mantissa % 2);
        a.exponent = a.exponent + 1;
    }

    uint32_t g = (mantissa >> 2) & 1;
    uint32_t r = (mantissa >> 1) & 1;
    uint32_t s = (mantissa & 1);

    mantissa /= 8;
    a.exponent += 3;

    if (g && (r | s | (mantissa & 1))) {
        mantissa++;
        if (mantissa == (1u << 24)) {
            mantissa >>= 1;
            a.exponent++;
        }
    }

    if (a.exponent >= 128) {
        mantissa = 0;
        a.exponent = 128;
    }

    // When the sum of two operands with opposite signs (or the difference of two operands with like signs) is exactly zero, the sign of that sum (or difference) shall be +0
    if (!mantissa)
        negative = false;
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

    return normalized({!a.negative != !b.negative, static_cast<int16_t>(a.exponent + b.exponent + 1), mantissa_high});
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

    return normalized({!a.negative != !b.negative, exponent, mantissa});
}

TinyFloat operator-(const TinyFloat &f) {
    if (f.isnan()/* || (f.mantissa==0 && f.exponent==-126)*/ ) return f;
//std::cerr << f << ": " << f.exponent << " " << f.mantissa << std::endl;
    return {!f.negative, f.exponent, f.mantissa}; // no normalization
}


