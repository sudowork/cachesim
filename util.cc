#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include "cache.h"
#include "util.h"

std::vector<std::string> util::splitLine(const std::string str, const char delim)
{
    std::vector<std::string> v;
    int last, i = -1;
    while (true) {
        last = i;
        i = str.find(delim,i+1);
        // Insert first non-empty token
        if (i-last > 1 || i == -1) v.push_back(str.substr(last+1,i-last-1));
        // break after hitting the end
        if (i == std::string::npos) break;
    }

    return v;
}

std::ostream& util::operator<< (std::ostream& out, const Cache& c)
{
    out << "Cache Description" << std::endl;
    out << "Cache-Size: " << c.getCacheSize() << " KB" << std::endl;
    out << "Associativity: " << c.getAssociativity() << "-way" << std::endl;
    out << "Block-Size: " << c.getBlockSize() << " B" << std::endl;
    out << "# Blocks: " << c.getNumBlocks() << std::endl;
    out << "# Sets: " << c.getNumSets();
    return out;
}

void util::padHex(std::ostream& out, char * val, const int bytes)
{
    for (int i = 0; i < bytes; i++) {
        uint32_t charval = static_cast<unsigned int>(val[i]);
        charval &= 0xff;
        out << std::setfill('0') << std::setw(2) << std::hex << charval;
    }
    return;
}
