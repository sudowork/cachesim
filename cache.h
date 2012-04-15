#ifndef CACHE_H
#define CACHE_H

#include <fstream>
#include <vector>

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
                _numSets(cs/(bs*a)) // Calculate number of sets
        {
        };
        ~Cache()
        {
            // Close file handler
            if (_fs.is_open()) _fs.close();
        };

        bool loadFile();
        bool loadFile(const char * f);
        void exec();

};

#endif
