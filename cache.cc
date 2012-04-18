#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <list>
#include "stdint.h"
#include <map>
#include "util.h"
#include "cache.h"

void Cache::loadFile()
{
    loadFile(_filename);
}

const bool Cache::loadFile(const char* filename)
{
    // Open file stream to read file
    try {
        _fs.open(filename);
        if (_fs == NULL) throw 20;
        return true;
    } catch (int e) {
        std::cout << std::endl
            << "Error (" << e << ") opening file `" << filename << "`" << std::endl;
        exit(e);
    }
}

void Cache::exec()
{
    // Ensure that file is open
    if (!_fs.is_open()) {
        std::cout << std::endl << "File not open" << std::endl;
        exit(21);
    }

    // Loop through lines of file
    while (_fs.good()) {
        // Get next line
        std::string line;
        getline(_fs,line);
        line = util::trim(line);

        // ignore blank lines
        if (line.size() == 0) continue;

        // TODO command parser?
        // Tokenize command
        std::vector<std::string> cmd = util::splitLine(line, ' ');

        try {
            // parse instruction
            std::string insn = cmd[0];
            std::transform(insn.begin(),insn.end(),insn.begin(),(int(*)(int))std::tolower);   // Convert to lowercase

            std::istringstream* ss;
            unsigned int address;
            unsigned short accessSize;
            // Check if appropriate number of parameters are listed
            if (((insn.compare("store") == 0 || insn.compare("load") == 0)
                    && cmd.size() >= 3) || insn.compare("//") == 0) {
                ss = new std::istringstream(cmd[1]);
                // Convert hex string of address to int
                *ss >> std::hex >> address;
                accessSize = FromString<unsigned short>(cmd[2].c_str());
            } else {
                throw 30;
            }

            // Check for commands manually
            if (insn.compare("store") == 0 && cmd.size() == 4) {
                // Convert hex srting to value buffer [MSB->LSB]
                char * value = new char[accessSize];
                for (int i = 0; i < accessSize; i++) {
                    char * hexbyte = new char[2];
                    hexbyte[0] = cmd[3][i*2];
                    hexbyte[1] = cmd[3][i*2+1];
                    value[i] = std::strtoul(hexbyte,0,16);
                }

                // Attempt to store
                CacheResult cr = this->store(address,accessSize,value);

                if (cr.hit) {
                    std::cout << "store hit" << std::endl;
                    //util::padHex(std::cout,cr.value,accessSize);
                    //std::cout << std::endl;
                } else {
                    std::cout << "store miss" << std::endl;
                }

            } else if (insn.compare("load") == 0 && cmd.size() == 3) {
                // Attempt to load
                CacheResult cr = this->load(address,accessSize);

                if (cr.hit) {
                    std::cout << "load hit ";
                    util::padHex(std::cout,cr.value,accessSize);
                    std::cout << std::endl;
                } else {
                    std::cout << "load miss ";
                    util::padHex(std::cout,cr.value,accessSize);
                    std::cout << std::endl;
                }
            } else if (insn.compare("//") == 0) {
                std::cout << line << std::endl;
            } else {
                throw 30;
            }
        } catch (int e) {
            std::cout << std::endl << "Invalid tracefile command:" << std::endl;
            std::cout << ">" << line << std::endl;
            exit(e);
        }
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

Cache::CacheResult Cache::store(unsigned int address, unsigned short accessSize, char * value)
{
    // Instantiate slot and return value
    Slot si;
    CacheResult cr;
    cr.hit = false;
    cr.value = new char[accessSize];

    // calculate block offset for use later
    const unsigned short blockOffset = address & OFF_BITMASK;

    // Find match (or get last item if no match)
    std::list<Slot> &s = sets[(address / _blockSize) % _numSets];  // (Block number) % numsets
    std::list<Slot>::iterator it = this->findMatch(s,address,cr,accessSize);
    // Get slot to be replaced
    si = *it;

    // Process index
    si.d = true;
    si.V = true;
    si.fields = address;
    // Update cache data
    std::copy(value,value+accessSize,si.data+blockOffset);

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

    // Find match (or get last item if no match)
    std::list<Slot> &s = sets[(address / _blockSize) % _numSets];
    std::list<Slot>::iterator it = this->findMatch(s,address,cr,accessSize);
    // Get slot to be replaced
    si = *it;

    // Process index
    si.d = false;
    si.V = true;
    si.fields = address;

    // load data from memory if not hit
    if (!cr.hit) {
        unsigned int blockNum = address / _blockSize;
        if (mainMem->count(blockNum) > 0) {
            char * blockFromMem = mainMem->at(blockNum);
            std::copy(blockFromMem+blockOffset,blockFromMem+blockOffset+accessSize,si.data+blockOffset);
            cr.value = si.data+blockOffset;
        }
        cr.value = si.data+blockOffset;
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
        // overwrite previous data
        char * newData = new char[_blockSize];  // separate cachemem from mainmem
        std::copy(toRemove.data,toRemove.data+_blockSize,newData);
        std::pair<int,char *> p = std::make_pair(blockNumber,newData);
        mainMem->insert(p);
    }
    s.erase(it);
    return;
}

std::list<Cache::Slot>::iterator Cache::findMatch(std::list<Slot> &s, const uint32_t address, CacheResult &cr, const unsigned short accessSize)
{
    const unsigned short blockOffset = address & OFF_BITMASK;

    // Look for matching tag
    uint32_t tag = address & TAG_BITMASK;

    std::list<Slot>::iterator it;
    for (it = s.begin(); it != s.end(); ++it) {
        Slot si = *it;    // Set to last iterator
        // XOR to find match
        if ((tag ^ (si.fields & TAG_BITMASK)) == 0x0 && si.V) {
            cr.hit = true;
            // Copy value from cache block to return block
            std::copy(si.data+blockOffset,si.data+blockOffset+accessSize,cr.value);
            break;
        }
    }

    // Remove current or last element
    // May perform write-back if necessary
    it = (cr.hit) ? it : --it;
    this->popSlot(s,it);

    return it;
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
