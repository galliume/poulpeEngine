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

    updateSkybox();
    /*
    *   message format is:
    *   functionName_param1_param2_etc
    *
    *   functionName is a string terminated by "_"
    *   each param start with the type coded on one byte:
    *     0: bool
    *     1: integer
    *     2: string
    *   then after the one byte the value until the next "_"
    * 
    *   message: loadSkybox_11
    *   function to call loadSkybox with 1 param of type 1 (integer) of value 1 (skybox id)
    * 
    *   last two bytes "\0" are discarded
    */
    //size_t funcNamePos = message.find("_");
    //std::string funcName = message.substr(0, funcNamePos);
    //PLP_TRACE("funcName: {}", funcName);

    //std::vector<std::variant<int, float, std::string>> params;

    //bool typeDone{ false };
    //bool paramDone{ false };
    //int type{ 0 };

    //std::vector<std::variant<int, float, std::string>> buffer;

    //for (auto const& c : message.substr(funcNamePos + 1, message.size())) {
    //  //if (!typeDone) {
    //  //  type = c - '0';//converts '0' to 0
    //  //  type = true;
    //  //} else if (c == '_') {
    //  //  typeDone = false;
    //  //  paramDone = false;

    //  //  std::variant<int, float, std::string> param;
    //  //  switch (type) {
    //  //  case 0:
    //  //    param = static_cast<bool>(buffer);
    //  //    break;
    //  //  case 1:
    //  //    param = static_cast<int>(buffer);
    //  //    break;
    //  //  case 2:
    //  //    param = static_cast<std::string>(buffer);
    //  //    break;
    //  //  }
    //  //  params.emplace_back(param);
    //  //} else if (!paramDone) {
    //  //  std::string str(c);
    //  //  buffer.append(str);
    //  //}
    //  PLP_TRACE("type: {}", type);
    //  PLP_TRACE("buffer: {}", buffer);
    //}
  }

  void APIManager::updateSkybox()
  {
    std::function<void()> request = [this]() {
      auto loading = m_RenderManager->getTextureManager()->loadSkybox("debug");

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
}
