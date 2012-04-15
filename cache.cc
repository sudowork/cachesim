#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cmath>
#include "util.h"
#include "cache.h"

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

                // Extract common parameters
                try {
                    // parse instruction
                    std::string insn = cmd[0];
                    std::transform(insn.begin(),insn.end(),insn.begin(),(int(*)(int))std::tolower);   // Convert to lowercase

                    int address = FromString<int>(cmd[1].c_str());
                    unsigned short accessSize = FromString<unsigned short>(cmd[2].c_str());

                    if (insn.compare("store") == 0) {
                        int value = FromString<int>(cmd[3].c_str());
                        store(address,accessSize,value);
                    } else if (insn.compare("load")) {
                        load(address,accessSize);
                    }
                } catch (int e) {
                    std::cout << "Invalid tracefile command" << std::endl;
                    return;
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

        // Add invalid slot to set's queue
        sets[i].push(si);
    }
}

const bool Cache::store(int address, unsigned short accessSize, int value)
{
    return false;
}

const bool Cache::load(int address, unsigned short accessSize)
{
    return false;
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

template<typename T>
T FromString(const char * str)
{
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}
