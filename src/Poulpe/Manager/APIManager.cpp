#include "APIManager.hpp"

namespace Poulpe
{
  void APIManager::received(std::string const& message)
  {
    PLP_TRACE("received: {}", message);
  }
}
