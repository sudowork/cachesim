#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include "cache.h"
#include "util.h"

bool Cache::loadFile()
{
    return loadFile(_filename);
}

bool Cache::loadFile(const char* filename)
{
    // Open file stream to read file
    try {
        _fs.open(filename);
        return true;
    } catch (int e) {
        std::cerr << "Error (" << e << ") opening file `" << filename << "`" << std::endl;
        throw 20;
    }
}

void Cache::exec()
{
    // Ensure that file is open
    if (_fs.is_open()) {
        std::cout << "Executing Contents of " << _filename << std::endl;
        while (_fs.good()) {
            std::string line;
            getline(_fs,line);
            if (line.size() > 0) {  // ignore blank lines
                std::vector<std::string> cmd = util::splitLine(line, ' ');
            }
        }
    } else {
        std::cerr << "File not open" << std::endl;
        throw 21;   // File not open
    }
}
