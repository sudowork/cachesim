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
                            util::padHex(std::cout,cr.value,accessSize);
                            std::cout << std::endl;
                        } else {
                            std::cout << "store miss" << std::endl;
                        }

                    } else if (insn.compare("load") == 0) {
                        // Attempt to load
                        CacheResult cr = load(address,accessSize);

                        if (cr.hit) {
                            std::cout << "load hit ";
                            util::padHex(std::cout,cr.value,accessSize);
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
            Slot si;
            si.V = 0;
            si.d = 0;

            // Initialize set field
            si.fields = (i << OFFWIDTH) & (SET_BITMASK);    // shift and mask

            // Initialize cache mem to 0
            si.data = new char[_blockSize];

            // Add invalid slot to set's queue
            sets[i].push_front(si);
        }
    }
}

Cache::CacheResult Cache::store(unsigned int address, unsigned short accessSize, int value)
{
    // Instantiate slot and return value
    Slot si;
    CacheResult cr;
    cr.hit = false;
    cr.value = new char[accessSize];

    // calculate block offset for use later
    const unsigned short blockOffset = address & OFF_BITMASK;

    // Look for matching tag
    uint32_t tag = address & TAG_BITMASK;
    // TODO refactor this into a isInSet method
    std::list<Slot> &s = sets[(address / _blockSize) % _numSets];  // (Block number) % numsets

    std::list<Slot>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0x0 && si.V) {
            cr.hit = true;
            // Copy value from cache block to return block
            std::copy(si.data+blockOffset,si.data+blockOffset+accessSize,cr.value);
            break;  // break out to maintain as current slot
        }
    }

    // Remove current or last element
    // and write-back
    it = (cr.hit) ? it : --it;
    this->popSlot(s,it);

    // Process index
    si.d = cr.hit; // set dirty flag if already in cache and valid
    si.V = true;
    si.fields = address;
    this->writeToCache(value,si.data,accessSize);   // write new data

    // push to front (most recently used)
    s.push_front(si);

    return cr;
}

Cache::CacheResult Cache::load(unsigned int address, unsigned short accessSize)
{
    Slot si;
    CacheResult cr;
    cr.hit = false;
    cr.value = new char[accessSize];

    // calculate block offset for use later
    const unsigned short blockOffset = address & OFF_BITMASK;

    // Look for matching tag
    uint32_t tag = address & TAG_BITMASK;
    std::list<Slot> &s = sets[(address / _blockSize) % _numSets];  // (Block number) % numsets

    std::list<Slot>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0x0 && si.V) {
            cr.hit = true;
            // Copy value from cache block to return block
            std::copy(si.data+blockOffset,si.data+blockOffset+accessSize,cr.value);
            return cr;
        }
    }

    // Remove last element if miss
    --it;
    this->popSlot(s,it);

    // Process index
    si.d = false;
    si.V = true;
    si.fields = address;

    // load data from memory
    unsigned int blockNum = address / _blockSize;
    if (mainMem->count(blockNum) > 0) {
        char * blockFromMem = mainMem->at(blockNum);
        std::copy(blockFromMem+blockOffset,blockFromMem+blockOffset+accessSize,si.data+blockOffset);
    }

    // push to front
    s.push_front(si);

    return cr;  // cr.hit = false;
}

void Cache::popSlot(std::list<Slot> &s, std::list<Slot>::iterator &it)
{
    Slot toRemove = *it;
    if (toRemove.V && toRemove.d) {
        int blockNumber = toRemove.fields / _blockSize;
        // Check if dirty; write-back if necessary
        if (toRemove.d) {
            // overwrite previous data
            std::pair<int,char *> p = std::make_pair(blockNumber,toRemove.data);
            mainMem->insert(p);
        }
    }
    s.erase(it);
    return;
}

void Cache::writeToCache(const int value, char * cacheBlock, const unsigned short accessSize)
{
    // Convert value to char buffer
    char * valuebuf = new char[accessSize];
    for (int i = 0; i < accessSize; i++) {
        unsigned short shamt = (accessSize-1-i)*8;
        uint32_t mask = 0xff << shamt;
        valuebuf[i] = ((value & mask) >> shamt);
    }
    // write data to cache
    std::copy(valuebuf,valuebuf+accessSize,cacheBlock);
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
