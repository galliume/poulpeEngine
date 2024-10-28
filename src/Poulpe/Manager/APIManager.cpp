#include "APIManager.hpp"

#include <variant>

namespace Poulpe
{
  APIManager::APIManager(IRenderManager* renderManager)
    : m_RenderManager(renderManager)
  {

  }
  void APIManager::received(std::string const& message)
  {
    PLP_TRACE("received: {}", message);

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
    PLP_TRACE("funcName: {}", funcName);
  
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
      PLP_ERROR("Unknown API func: {}", funcName);
    }
  }

  void APIManager::updateSkybox(std::vector<std::string> const & params)
  {
    if (params.size() == 0 || params.size() > 1) return;

    std::string skyboxName = params.at(0);

    std::function<void()> request = [this, skyboxName]() {
      PLP_TRACE("skyboxName: {}", skyboxName);
      std::latch count_down{ 1 };

      std::jthread textures(std::move(std::bind(m_RenderManager->getTextureManager()->loadSkybox(skyboxName), std::ref(count_down))));
      textures.detach();
      count_down.wait();

      auto skybox = m_RenderManager->getEntityManager()->getSkybox();
      auto meshComponent = m_RenderManager->getComponentManager()->getComponent<MeshComponent>(skybox->getID());
      auto meshRenderer = m_RenderManager->getComponentManager()->getComponent<RenderComponent>(skybox->getID());

      Mesh* mesh = meshComponent->hasImpl<Mesh>();

      if (!mesh) return;
      mesh->setIsDirty(true);
      mesh->getData()->m_TextureIndex = 1;
      
      auto deltaTime = std::chrono::duration<float, std::milli>(0);
      //@todo add update to not re create the whole mesh...
      meshRenderer->visit(deltaTime, mesh);
    };

    Command cmd{request};
    Poulpe::Locator::getCommandQueue()->add(cmd);
  }
}
