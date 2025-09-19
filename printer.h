#include <iostream>
#include <cstdint>

struct Q128_149 {
    int number[277] = {};
    Q128_149(int offset, uint32_t n);
    bool print(std::ostream& out, bool trailing=true, int carry=0, int digno=0) const;
};

std::ostream& operator<<(std::ostream& out, const Q128_149& f);

