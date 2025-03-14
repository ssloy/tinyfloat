#include <iostream>
#include <cmath>
#include <iomanip>
#include <limits>
#include <random>
#include <cstdlib>

#include "tinyfloat.h"

TinyFloat unpack(float f) {
    const uint32_t u = std::bit_cast<uint32_t>(f);
    int sign_bit     = (u >> 31) % 2;
    int raw_exponent = (u >> 23) % 256;
    int raw_mantissa =  u % 0x800000;
    return { sign_bit == 1, static_cast<int8_t>(raw_exponent - 127), static_cast<uint32_t>(raw_mantissa + 0x800000*int(raw_exponent!=0)) };
}

int main() {
    float f1 = M_PI;
    TinyFloat f2 = unpack(f1);
    std::cerr << std::fixed << std::setprecision(std::numeric_limits<float>::max_digits10) << f1 << std::endl;
    std::cerr << f2 << std::endl;

    return 0;
}

