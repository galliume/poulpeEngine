module;

#include <latch>
#include <ranges>
#include <variant>

export module Poulpe.Manager.APIManager;

import Poulpe.Manager.RenderManager;

namespace Poulpe
{
  export class APIManager
  {
  public:
    APIManager(RenderManager* renderManager);
    ~APIManager()  = default;

    void received(std::string const& message);

  private:
    void updateSkybox(std::vector<std::string> const & params);

  private:
    RenderManager* _render_manager{ nullptr };
  };
}
