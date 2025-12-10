export module Engine.Core.Tools;

import std;

namespace Poulpe
{
  export class Tools
  {
  public:
      static std::vector<char> readFile(const std::string & filename);
  };
}
