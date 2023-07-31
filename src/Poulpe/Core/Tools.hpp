#pragma once
#include <string>

namespace Poulpe
{
    class Tools
    {
    public:
        static std::vector<char> ReadFile(const std::string& filename);
    };
}
