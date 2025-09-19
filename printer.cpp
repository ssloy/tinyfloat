#include "printer.h"

Q128_149::Q128_149(int offset, uint32_t n) {
    while (n > 0) {
        number[offset++] = n % 2;
        n = n / 2;
    }
}

bool Q128_149::print(std::ostream& out, bool trailing, int carry, int digno) const {
    int sum = 0;
    for (int i=0; i<nbits; i++)
        sum += number[i] * digits[digno][i];
    int digit = (sum + carry) % 10;
    bool leading = !digit && digno > dotpos;
    trailing    &= !digit && digno < dotpos-1;
    if (digno < ndigits-1)
        leading &= print(out, trailing, (sum + carry)/10, digno + 1);
    if (digno == dotpos-1) out << ".";
    if (!leading && !trailing) out << digit;
    return leading;
}

std::ostream& operator<<(std::ostream& out, const Q128_149& f) {
    f.print(out);
    return out;
}


