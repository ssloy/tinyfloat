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
    while (mantissa >= (1<<24)  && exponent < 128) { // TODO decide what to do if the normalization does not work
        mantissa = mantissa / 2;
        exponent = exponent + 1;
    }
}

TinyFloat::TinyFloat(int i) {
    *this = TinyFloat(i<0, 23, i<0?-i:i);
}

TinyFloat::TinyFloat(float f) {
    const uint32_t u = std::bit_cast<uint32_t>(f);
    int sign_bit     = (u >> 31) % 2;
    int raw_exponent = (u >> 23) % 256;
    int raw_mantissa =  u % 0x800000;
    *this = { sign_bit == 1, static_cast<int16_t>(raw_exponent - 127), static_cast<uint32_t>(raw_mantissa + 0x800000*int(raw_exponent!=0)) };
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
//  std::cerr <<  lhs.exponent << " " << rhs.exponent << " " << lhs.mantissa << " " << rhs.mantissa<< std::endl;
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
        if (a.mantissa>=b.mantissa) {
            mantissa = a.mantissa - b.mantissa;
        } else {
            negative = !a.negative;
            mantissa = b.mantissa - a.mantissa;
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

