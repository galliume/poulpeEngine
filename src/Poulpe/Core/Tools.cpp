#include "Tools.hpp"

namespace Poulpe
{
    std::vector<char> Tools::readFile(const std::string & filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        std::streamsize fileSize = file.tellg();
        std::vector<char> buffer(static_cast<unsigned long>(fileSize));

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }
}
