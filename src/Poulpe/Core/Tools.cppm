module;

#include <string>
#include <vector>

export module Poulpe.Core.Tools;

export class Tools
{
public:
    static std::vector<char> readFile(const std::string & filename);
};
