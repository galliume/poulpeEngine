#pragma once

#include "IAPIManager.hpp"

#include "IRenderManager.hpp"

namespace Poulpe
{
  class APIManager : public IAPIManager
  {
  public:
    APIManager(IRenderManager* renderManager);
    ~APIManager() override = default;

    void received(std::string const& message) override;

  private:
    void updateSkybox(std::vector<std::string> const & params);
    void updateLevel(std::vector<std::string> const & params);

  private:
    IRenderManager* m_RenderManager{ nullptr };
  };
}
