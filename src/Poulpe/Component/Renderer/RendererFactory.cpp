module Poulpe.Components.Renderer;

static std::unique_ptr<T> RendererFactory::create(TArgs&&... args)
{
  return std::make_unique<T>(T(std::forward<TArgs>(args)...));
}
