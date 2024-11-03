#pragma once

#include "Poulpe/Manager/RenderManager.hpp"

namespace Poulpe
{
  class APIManager
  {
  public:
    APIManager(RenderManager* renderManager);
    ~APIManager()  = default;

    void received(std::string const& message);

  private:
    void updateSkybox(std::vector<std::string> const & params);

  private:
    RenderManager* m_RenderManager{ nullptr };
  };
}
