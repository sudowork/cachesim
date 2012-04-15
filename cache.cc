#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cmath>
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
        std::cout << "Error (" << e << ") opening file `" << filename << "`" << std::endl;
        return false;
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
                // Tokenize command
                std::vector<std::string> cmd = util::splitLine(line, ' ');

                // TODO command parser?
                // parse instruction
                std::string insn = cmd[0];
                std::transform(insn.begin(),insn.end(),insn.begin(),(int(*)(int))std::tolower);   // Convert to lowercase

                if (insn.compare("store")) {

                } else if (insn.compare("load")) {

                }
            }
        }
    } else {
        std::cerr << "File not open" << std::endl;
        throw 21;   // File not open
    }
}

void Cache::initSets() {
    for (int i = 0; i < _numSets; i++) {
        Index si;
        si.V = 0;
        si.d = 0;
        // Insert set number (offset past block-offset)
        // Calculate widths of each field
        unsigned short OFFWIDTH = log(_blockSize)/log(2);
        unsigned short SETWIDTH = log(_numSets)/log(2);
        unsigned short TAGWIDTH = BUSWIDTH-OFFWIDTH-SETWIDTH;
        // Bitmask so only SET can be modified (just in case)
        int BITMASK = ~(0xffffffff & (0x0 << OFFWIDTH + SETWIDTH));
        si.fields = (i << OFFWIDTH) & (BITMASK);    // shift and mask
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
const unsigned short Cache::getNumBlocks() const
{
    return this->_numBlocks;
}
