#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include "cache.h"
#include "util.h"

const bool Cache::loadFile()
{
    return loadFile(_filename);
}

const bool Cache::loadFile(const char* filename)
{
    // Open file stream to read file
    try {
        _fs.open(filename);
        return true;
    } catch (int e) {
        std::cerr << "Error (" << e << ") opening file `" << filename << "`" << std::endl;
        throw 20;
    }
}

void Cache::exec()
{
    // Ensure that file is open
    if (_fs.is_open()) {
        while (_fs.good()) {
            std::string line;
            getline(_fs,line);
            if (line.size() > 0) {  // ignore blank lines
                std::vector<std::string> cmd = util::splitLine(line, ' ');
            }
        }
    } else {
        std::cerr << "File not open" << std::endl;
        throw 21;   // File not open
    }
}

const unsigned short Cache::getCacheSize() const
{
    return this->_cacheSize;
}
const unsigned short Cache::getAssociativity() const
{
    return this->_associativity;
}
const unsigned short Cache::getBlockSize() const
{
    return this->_blockSize;
}
const unsigned short Cache::getNumSets() const
{
    return this->_numSets;
}
