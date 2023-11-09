#pragma once
#include <string>

namespace Poulpe
{
    class Tools
    {
    public:
        static std::vector<char> readFile(const std::string& filename);
    };
}
