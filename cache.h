#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <ostream>
#include <cstdint>
//#include <unordered_map>
#include <list>
#include <cmath>

#define BUSWIDTH 32

class Cache
{
    private:
        const char * _filename;
        std::ifstream _fs;
        // Different parameters
        const unsigned short _cacheSize,
              _associativity,
              _blockSize,
              _numBlocks,
              _numSets,
              OFFWIDTH,
              SETWIDTH,
              TAGWIDTH;
        // 32-bit masks for fields
        const uint32_t OFF_BITMASK,
              TAG_BITMASK,
              SET_BITMASK;

        typedef struct
        {
            /* --------------------------------
             * |V|d|  TAG  |  SET  |  OFFSET  |
             * --------------------------------
             *  V (valid) = 1-bit
             *  d (dirty) = 1-bit
             *  TAG = BUSWIDTH-log2(numSets)-log2(blockSize)
             *  SET = log2(numSets)
             *  OFFSET = log2(blockSize)
             */
            bool V;
            bool d;
            uint32_t fields;
        } Index;

        /*typedef struct
        {
            char* data;
        } Block;*/

        // array of lists (front of list = most recently used)
        std::list<Index> *sets;
        char * cacheMem;
        //std::unordered_map<uint32_t,Block> cacheMem;

        void init();

    public:
        // Constructor/Destructor
        Cache(const char * f, const unsigned short cs,
                const unsigned short a, const unsigned short bs)
            :   _filename(f),
                _cacheSize(cs),
                _associativity(a),
                _blockSize(bs),
                _numBlocks(cs*1024/bs),
                _numSets(_numBlocks/a), // Calculate number of sets
                OFFWIDTH(log(_blockSize)/log(2)),
                SETWIDTH(log(_numSets)/log(2)),
                TAGWIDTH(BUSWIDTH-OFFWIDTH-SETWIDTH),
                OFF_BITMASK(~((0xffffffff >> OFFWIDTH) << OFFWIDTH)),
                TAG_BITMASK((0xffffffff >> (OFFWIDTH + SETWIDTH)) << (OFFWIDTH + SETWIDTH)),
                SET_BITMASK(~(0xffffffff & (OFF_BITMASK | TAG_BITMASK))),
                sets(new std::list<Index>[_numSets]),
                cacheMem(new char[cs*1024])
        {
            init();
        };
        ~Cache()
        {
            delete[] sets;
            delete[] cacheMem;
            // Close file handler
            if (_fs.is_open()) _fs.close();
        };

        // getters
        const unsigned short getCacheSize() const;
        const unsigned short getAssociativity() const;
        const unsigned short getBlockSize() const;
        const unsigned short getNumBlocks() const;
        const unsigned short getNumSets() const;

        const bool loadFile();
        const bool loadFile(const char * f);
        void exec();
        const bool store(unsigned int address, unsigned short accessSize, int value);
        const bool load(unsigned int address, unsigned short accessSize);


};

// For some reason, this wasn't letting me put it in the util namespace
template <typename T>
T FromString(const char * str);

#endif
