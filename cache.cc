#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <list>
#include <cstdint>
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

                    std::istringstream* ss = new std::istringstream(cmd[1]);
                    unsigned int address;
                    *ss >> std::hex >> address;
                    unsigned short accessSize = FromString<unsigned short>(cmd[2].c_str());

                    if (insn.compare("store") == 0) {
                        int value;
                        delete ss;
                        ss = new std::istringstream(cmd[3]);
                        *ss >> std::hex >> value;

                        if (store(address,accessSize,value)) {
                            // TODO retrieve previous value??
                            std::cout << "store hit " << std::endl;
                        } else {
                            std::cout << "store miss" << std::endl;
                        }
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

void Cache::init() {
    // Initialize sets to null (add all slots)
    for (int i = 0; i < _numSets; ++i) {
        for (int j = 0; j < _associativity; ++j) {
            Index si;
            si.V = 0;
            si.d = 0;

            // Initialize set field
            si.fields = (i << OFFWIDTH) & (SET_BITMASK);    // shift and mask

            // Add invalid slot to set's queue
            sets[i].push_front(si);
        }
    }

    // zero out cacheMem
    // Doesn't really matter
    std::fill_n(cacheMem,_cacheSize*1024,0);
}

const bool Cache::store(unsigned int address, unsigned short accessSize, int value)
{
    std::list<Index> &s = sets[address % _numSets];

    // Get most significant bits
    uint32_t tag = address & TAG_BITMASK;

    Index si;
    bool hit = false;

    // Look for matching tag
    std::list<Index>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0) {
            hit = true;
            break;
        }
    }

    // Remove current or last element
    s.erase((hit) ? it : --it);

    uint32_t set = si.fields & SET_BITMASK;
    uint32_t offset = (_blockSize-accessSize) & OFF_BITMASK;

    // Process index
    si.d = hit and si.V; // set dirty flag if already in cache and valid
    si.V = true;
    si.fields = 0 + tag + set + offset;

    // push to front
    s.push_front(si);

    return hit and si.V;
}

const bool Cache::load(unsigned int address, unsigned short accessSize)
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
