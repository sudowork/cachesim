#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <ostream>

class Cache
{
    private:
        const char * _filename;
        const unsigned short _cacheSize,
              _associativity,
              _blockSize,
              _numSets;
        std::ifstream _fs;

    public:
        // Constructor/Destructor
        Cache(const char * f, const unsigned short cs,
                const unsigned short a, const unsigned short bs)
            :   _filename(f),
                _cacheSize(cs),
                _associativity(a),
                _blockSize(bs),
                _numSets(cs*1024/(bs*a)) // Calculate number of sets
        {
        };
        ~Cache()
        {
            // Close file handler
            if (_fs.is_open()) _fs.close();
        };

        // getters
        const unsigned short getCacheSize() const;
        const unsigned short getAssociativity() const;
        const unsigned short getBlockSize() const;
        const unsigned short getNumSets() const;

        const bool loadFile();
        const bool loadFile(const char * f);
        void exec();

};

#endif
