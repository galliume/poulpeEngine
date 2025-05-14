export module Poulpe.Components.Renderer:RendererFactory;

import Basic;
import Crosshair;
import Grid;
import Skybox;
import Terrain;
import Text;
import Water;

export class RendererFactory
{
public:
  template <typename T, typename... TArgs>
  static std::unique_ptr<T> create(TArgs&&... args);
};
