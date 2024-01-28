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
    *   each param end with "_"   * 
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
    } else if (funcName == "updateLevel") {
      updateLevel(params);
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
      auto loading = m_RenderManager->getTextureManager()->loadSkybox(skyboxName);

      loading();

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

  void APIManager::updateLevel(std::vector<std::string> const & params)
  {
    if (params.size() == 0 || params.size() > 1) return;

    std::string level = params.at(0);

    std::function<void()> request = [this, level]() {

      auto levels = m_RenderManager->getConfigManager()->listLevels();
      uint32_t index{ 0 };

      auto it = std::find(levels.begin(), levels.end(), level);
    
      if (it != levels.end()) {
        index = static_cast<uint32_t>(std::distance(levels.begin(), it));

        m_RenderManager->getRenderer()->waitIdle();
        m_RenderManager->refresh(index);
      } else {
        PLP_WARN("Level {} not found", level);
      }
    };

    Command cmd{request, WhenToExecute::POST_RENDERING};
    Poulpe::Locator::getCommandQueue()->add(cmd);
  }
}
