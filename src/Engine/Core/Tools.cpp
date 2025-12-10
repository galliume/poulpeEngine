module Engine.Core.Tools;

import std;

import Engine.Core.Logger;

namespace Poulpe
{
  std::vector<char> Tools::readFile(const std::string & filename)
  {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
      Logger::error("failed to open file: {}", filename);
      return {};
    }

    std::streamsize file_size = file.tellg();
    std::vector<char> buffer(static_cast<unsigned long>(file_size));

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();

    return buffer;
  }
}
