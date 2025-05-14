export module Poulpe.Manager:APIManager;

import RenderManager;

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
