#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <ostream>
#include <cinttypes>
#include <unordered_map>
#include <queue>

#define BUSWIDTH 32

class Cache
{
    private:
        const char * _filename;
        const unsigned short _cacheSize,
              _associativity,
              _blockSize,
              _numBlocks,
              _numSets;
        std::ifstream _fs;

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

        typedef struct
        {
            char* data;
        } Block;

        std::queue<Index> *sets;
        std::unordered_map<uint32_t,Block> cacheMem;

        void initSets();

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
                sets(new std::queue<Index>[_numSets])
        {
            initSets();
        };
        ~Cache()
        {
            delete[] sets;
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

};

#endif
