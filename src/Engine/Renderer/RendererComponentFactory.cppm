module;
#include <memory>

export module Engine.Renderer.RendererComponentFactory;

namespace Poulpe
{
  export class RendererComponentFactory
  {
  public:
    template <typename T, typename... TArgs>
    static std::unique_ptr<T> create(TArgs&&... args)
    {
      return std::make_unique<T>(std::forward<TArgs>(args)...);
    }
  };
}
