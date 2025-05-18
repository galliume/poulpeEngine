module;
#include <functional>
#include <latch>
#include <string>
#include <thread>

module Poulpe.Managers;

import Poulpe.Core.Logger;

namespace Poulpe
{
  void APIManager::received(std::string const& message)
  {
    Logger::trace("received: {}", message);

    /*
    *   @todo: improve first naive draft
    *
    *   message format is:
    *   functionName_param1_param2_etc
    *
    *   functionName is a string terminated by "_"
    *   each param end with "_"
    *
    *   message: loadSkybox_bluesky
    *   function to call loadSkybox with param bluesky
    */
    size_t funcNamePos = message.find("_");
    std::string funcName = message.substr(0, funcNamePos);
    Logger::trace("funcName: {}", funcName);

    std::vector<std::string> params;
    std::string buffer;

    for (auto const& c : message.substr(funcNamePos + 1, message.size())) {
      if (c == '_') {
        params.emplace_back(buffer);
        buffer.clear();
      } else {
        buffer += c;
      }
    }

    params.emplace_back(buffer);

    if (funcName == "updateSkybox") {
      updateSkybox(params);
    } else {
      Logger::error("Unknown API func: {}", funcName);
    }
  }

  void APIManager::updateSkybox(std::vector<std::string> const & params)
  {
    //@todo should not be using _render_manager
    // if (params.size() == 0 || params.size() > 1) return;

    // std::string skyboxName = params.at(0);

    // std::function<void()> request = [this, skyboxName]() {
    //   Logger::trace("skyboxName: {}", skyboxName);
    //   std::latch count_down{ 1 };

    //   //@todo why threaded ?...
    //   std::jthread textures(std::move(std::bind(_render_manager->getTextureManager()->loadSkybox(skyboxName), std::ref(count_down))));
    //   textures.detach();
    //   count_down.wait();

    //   auto skybox = _render_manager->getEntityManager()->getSkybox();
    //   auto* mesh_component = _render_manager->getManager()->get<MeshComponent>(skybox->getID());
    //   auto* meshRenderer = _render_manager->getManager()->get<RenderComponent>(skybox->getID());

    //   if (mesh_component != nullptr) return;

    //   auto* mesh = mesh_component->has<Mesh>();

    //   if (!mesh) return;
    //   mesh->setIsDirty(true);
    //   mesh->getData()->_texture_index = 1;

    //   double const delta_time{ 0.0 };
    //   //@todo add update to not re create the whole mesh...
    //   (*meshRenderer)(delta_time, mesh);
    //};

    //Command cmd{request};
    //Locator::getCommandQueue()->add(cmd);
  }
}
