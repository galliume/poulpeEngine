module;
#include <memory>

export module Poulpe.Component.Renderer.RendererFactory;

namespace Poulpe
{
  export class RendererFactory
  {
  public:
    template <typename T, typename... TArgs>
    static std::unique_ptr<T> create(TArgs&&... args)
    {
      return std::make_unique<T>(T(std::forward<TArgs>(args)...));
    }
  };
}
