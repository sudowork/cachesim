/*
 * CacheSim
 * @author Kevin Gao [kag45], Oliver Fang [orf2]
 * Simulates an LRU cache
 */

#include <iostream>
#include <sstream>
#include "cache.h"
#include "util.h"
#include "cachesim.h"

int main(int argc, const char * argv[])
{
    using util::operator<<;

    // Check for valid input
    if (argc < 5)
    {
        std::cout << "Invalid number of parameters.\n"
            "Run as `cachesim <filename> <cache-size> <associativity> <block-size>`" << std::endl;
        return 1;
    }

    // Input parameters
    char * filename = NULL;
    unsigned short cacheSize,       // Cache size in KB
                   associativity,   // Level of associativity
                   blockSize;       // Block size in bytes
    try
    {
        // Cast and assign inputs
        filename = const_cast<char *>(argv[1]);
        cacheSize = FromString<unsigned short>(argv[2]);
        associativity = FromString<unsigned short>(argv[3]);
        blockSize = FromString<unsigned short>(argv[4]);
    } catch (int e) {
        std::cout << "Invalid Input: " << e << std::endl;
    }

    // Create new cache (and load file upon construction)
    Cache* c = new Cache(filename,cacheSize,associativity,blockSize);
    std::cout << *c << std::endl;   // Prints description of cache
    c->loadFile();

    std::cout << std::endl;
    // Execute stores and loads in tracefile
    c->exec();

    // Cleanup
    return 0;
}

template<typename T>
T FromString(const char * str)
{
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}
