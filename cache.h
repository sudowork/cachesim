#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <ostream>
#include <cstdint>
#include <unordered_map>
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

        // main memory
        // TODO separate this out into its own class
        std::unordered_map<int,char *> * mainMem;

        typedef struct
        {
            /* -----------------------------------------------------
             * |V|d|  TAG  |  SET  |  OFFSET  |        DATA        |
             * -----------------------------------------------------
             *  V (valid) = 1-bit
             *  d (dirty) = 1-bit
             *  TAG = BUSWIDTH-log2(numSets)-log2(blockSize)
             *  SET = log2(numSets)
             *  OFFSET = log2(blockSize)
             */
            bool V;
            bool d;
            uint32_t fields;
            char * data;    // block
        } Slot;

        typedef struct
        {
            bool hit;
            char * value;
        } CacheResult;

        // array of lists (front of list = most recently used)
        std::list<Slot> *sets;

        /**
         * Initializes the sets to contain n slots, where
         * the cache is an n-way set-associative cache
         *
         * Also initializes cache memory to 0's
         *
         * @return void
         */
        void init();

        /**
         * Stores a value in cache memory in the appropriate
         * set and block.
         *
         * @return CacheResult which contains a bool for hit
         *  and a value (0 if miss, cached value if hit)
         */
        CacheResult store(unsigned int address, unsigned short accessSize, char* value);

        /**
         * Loads data from cache, or attempts to fetch from
         * main memory if cache miss.
         *
         * @return CacheResult
         */
        CacheResult load(unsigned int address, unsigned short accessSize);

        /**
         * Removes a slot from a given set. Also checks if 
         * the slot was marked as dirty. If dirty, then
         * will write back to mainMemory.
         *
         * Side effect is that s will have one less item
         * and it will be invalidated
         *
         * @return void
         */
        void popSlot(std::list<Slot> &s, std::list<Slot>::iterator &it);

        /**
         * Either finds a matching tag within a set, or will
         * return the last item in the set. Calls popSlot
         * to remove the relevant slot.
         *
         * Side effects include setting the CacheResult
         * hit and value appropriately.
         *
         * @return std::list<Slot>::iterator
         */
        std::list<Slot>::iterator findMatch(std::list<Slot> &s, const uint32_t address, CacheResult &cr, const unsigned short accessSize);

    public:
        /**
         * Cache constructor. Instantiates new cache
         * based on cache size, associativity, and
         * block size. Calls init on self after
         * instiantiating member variables.
         */
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
                sets(new std::list<Slot>[_numSets]),
                mainMem(new std::unordered_map<int,char *>)
        {
            this->init();
        };
        /**
         * Cache destructor
         */
        ~Cache()
        {
            delete[] sets;
            free(mainMem);
            // Close file handler
            if (_fs.is_open()) _fs.close();
        };

        // getters
        const unsigned short getCacheSize() const;
        const unsigned short getAssociativity() const;
        const unsigned short getBlockSize() const;
        const unsigned short getNumBlocks() const;
        const unsigned short getNumSets() const;

        /**
         * Calls loadFile(const char*) with instantiated filename
         *
         * Otherwise equivalent to loadFile(_fileName)
         *
         * @return void
         */
        void loadFile();

        /**
         * Opens a filestream as a member variable _fs
         *
         * @return bool true if file is found
         */
        const bool loadFile(const char * f);

        /**
         * After tracefile has been loaded with loadFile() call,
         * exec() loops through the tracefile, decodes the 
         * instructions (strings to parameters), and calls the
         * appropriate methods based on the type of instruction.
         *
         * Currently supports only store, load, and comments.
         * 
         * Comments (lines beginning with "//", excluding leading
         * whitespace) are printed to STDOUT
         *
         * @return void
         */
        void exec();

};

// For some reason, this wasn't letting me put it in the util namespace
template <typename T>
T FromString(const char * str);

#endif
