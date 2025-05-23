module;

#include <string>
#include <vector>

export module Poulpe.Core.Tools;

namespace Poulpe
{
  export class Tools
  {
  public:
      static std::vector<char> readFile(const std::string & filename);
  };
}
