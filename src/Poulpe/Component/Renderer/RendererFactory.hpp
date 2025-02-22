#pragma once

#include "Basic.hpp"
#include "Crosshair.hpp"
#include "Grid.hpp"
#include "Skybox.hpp"
#include "Terrain.hpp"
#include "Text.hpp"
#include "Water.hpp"

namespace Poulpe
{
  class RendererFactory
  {
  public:
    template <typename T, typename... TArgs>
    static std::unique_ptr<T> create(TArgs&&... args)
    {
      return std::make_unique<T>(T(std::forward<TArgs>(args)...));
    }
  };
}
