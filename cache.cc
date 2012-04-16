#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <list>
#include <cstdint>
#include <unordered_map>
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
                    // Convert hex string of address to int
                    unsigned int address;
                    *ss >> std::hex >> address;
                    unsigned short accessSize = FromString<unsigned short>(cmd[2].c_str());

                    // Check for commands manually
                    if (insn.compare("store") == 0) {
                        // Convert hex string of value to int
                        int value;
                        delete ss;
                        ss = new std::istringstream(cmd[3]);
                        *ss >> std::hex >> value;

                        // Attempt to store
                        CacheResult cr = store(address,accessSize,value);

                        if (cr.hit) {
                            std::cout << "store hit ";
                            util::padHex(std::cout,cr.value,accessSize*2);
                            std::cout << std::endl;
                        } else {
                            std::cout << "store miss" << std::endl;
                        }

                    } else if (insn.compare("load") == 0) {
                        // Attempt to load
                        CacheResult cr = load(address,accessSize);

                        if (cr.hit) {
                            std::cout << "load hit ";
                            util::padHex(std::cout,cr.value,accessSize*2);
                            std::cout << std::endl;
                        } else {
                            std::cout << "load miss" << std::endl;
                        }
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

Cache::CacheResult Cache::store(unsigned int address, unsigned short accessSize, int value)
{
    // Get most significant bits
    uint32_t tag = address & TAG_BITMASK;

    Index si;
    CacheResult cr;
    cr.hit = false;
    cr.value = 0x0;

    // Look for matching tag
    // TODO refactor this into a isInSet method
    std::list<Index> &s = sets[(address % _numBlocks) % _numSets];  // (Block number) % numsets
    std::list<Index>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0x0 && si.V) {
            cr.hit = true;
            // Pass in set and offset
            cr.value = this->getFromCache((si.fields & SET_BITMASK) >> OFFWIDTH,(si.fields & OFF_BITMASK));
            break;
        }
    }

    // Remove current or last element
    // and write-back
    it = (cr.hit) ? it : --it;
    this->popIndex(s,it);

    uint32_t set = si.fields & SET_BITMASK;
    uint32_t offset = (_blockSize-accessSize) & OFF_BITMASK;

    // Process index
    si.d = cr.hit; // set dirty flag if already in cache and valid
    si.V = true;
    si.fields = 0x0 + tag + set + offset;

    // push to front
    s.push_front(si);

    // write value to blocks in cacheMem
    this->writeToCache(
            (si.fields & SET_BITMASK) >> OFFWIDTH,
            (si.fields & OFF_BITMASK),
            value);

    return cr;
}

Cache::CacheResult Cache::load(unsigned int address, unsigned short accessSize)
{
    // Get most significant bits
    uint32_t tag = address & TAG_BITMASK;

    Index si;
    CacheResult cr;
    cr.hit = false;
    cr.value = 0x0;

    // Look for matching tag
    std::list<Index> &s = sets[(address % _numBlocks) % _numSets];  // (Block number) % numsets
    std::list<Index>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0x0 && si.V) {
            cr.hit = true;
            cr.value = this->getFromCache((si.fields & SET_BITMASK) >> OFFWIDTH,(si.fields & OFF_BITMASK));
            return cr;
        }
    }

    // Remove last element if miss
    --it;
    this->popIndex(s,it);

    uint32_t set = si.fields & SET_BITMASK;
    uint32_t offset = (_blockSize-accessSize) & OFF_BITMASK;

    // Process index
    si.d = false; // ?
    si.V = true;
    si.fields = 0x0 + tag + set + offset;

    // push to front
    s.push_front(si);

    // write value to blocks in cacheMem (if exists in memory)
    unsigned int decodedTag = (si.fields & ~OFF_BITMASK) >> OFFWIDTH;
    if (mainMem->count(decodedTag) > 0) {
        this->writeToCache(
                (si.fields & SET_BITMASK) >> OFFWIDTH,
                (si.fields & OFF_BITMASK),
                mainMem->at(decodedTag));   // overloaded call
    }

    return cr;  // cr.hit = false;
}

const int Cache::getFromCache(const int set, const int offset)
{
    int ret = 0x0;
    for (int i = 0; i < _blockSize-offset; ++i) {
        int currByte = *(cacheMem+set*_associativity+offset+i);
        currByte &= 0xff;
        ret |= currByte << (i*8);
    }
    return ret;
}

void Cache::popIndex(std::list<Index> &s, std::list<Index>::iterator &it)
{
    Index toRemove = *it;
    if (toRemove.V && toRemove.d) {
        int decodedTag = (toRemove.fields & ~OFF_BITMASK) >> OFFWIDTH;
        // Has been modified
        // So retrieve from cacheMem and write-back
        uint32_t cmoffset =
            (((toRemove.fields & SET_BITMASK) >> OFFWIDTH)*_associativity);
        char * retrieved = new char[_blockSize];
        std::copy(cacheMem+cmoffset,cacheMem+cmoffset+_blockSize,retrieved);

        // overwrite previous data
        std::pair<int,char *> p = std::make_pair(decodedTag,retrieved);
        mainMem->insert(p);
    }
    s.erase(it);
    return;
}

// Write to cache from a store
void Cache::writeToCache(const int set, const int offset, const int value)
{
    uint32_t absOffset = set*_associativity+offset;
    for (int i = 0; i < _blockSize-offset; i++) {
        unsigned short shamt = i*8;
        uint32_t mask = 0xff << shamt;
        cacheMem[absOffset+i] = (value & mask) >> shamt;
    }
    return;
}

// Write to cache from a load
// (using reference to char buffer in main mem)
void Cache::writeToCache(const int set, const int offset, char * value)
{
    uint32_t absOffset = set*_associativity+offset;
    int i = 0;
    // increment pointer for up to blockSize (prevent null pointer exception)
    while (value+i != value+_blockSize) {
        if (value+i == NULL) {
            --i;
            break;
        }
        ++i;
    }

    std::copy(value,value+i,cacheMem+absOffset);
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
