#ifndef CHEX_H
#define CHEX_H

#include <iostream>
#include <iomanip>

namespace std {
    template <typename T>
    void chex(const T& value) {
        std::cout << std::hex << std::uppercase << value << std::dec << std::endl;
    }
}

#endif // CHEX_H
