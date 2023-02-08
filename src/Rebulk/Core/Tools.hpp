#pragma once
#include <string>

namespace Rbk
{
    class Tools
    {
    public:
        static std::vector<char> ReadFile(const std::string& filename);
    };
}
