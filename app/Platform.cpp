#include "Platform.h"

#include <fstream>
#include <sstream>

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

