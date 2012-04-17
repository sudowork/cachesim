#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <ostream>
#include "cache.h"

namespace util {
    std::vector<std::string> splitLine(const std::string str, const char delim);
    std::ostream& operator<< (std::ostream& out, const Cache& c);
    void padHex(std::ostream& out, char * value, const int bytes);
}

#endif
