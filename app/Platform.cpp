#include "Platform.h"

#include <fstream>
#include <sstream>

std::string toLinuxStyle(const std::filesystem::path& p)
{
    std::string s = p.string();
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

