#pragma once

#include "IAPIManager.hpp"

namespace Poulpe
{
  class APIManager : public IAPIManager
  {
  public:
    APIManager() = default;
    ~APIManager() override = default;

    void received(std::string const& message) override;
  private:

  };
}
