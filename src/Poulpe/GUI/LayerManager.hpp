#pragma once
#include "Poulpe/GUI/Layer/ILayer.hpp"

namespace Poulpe
{
    class LayerManager
    {
    public:
        ~LayerManager();
        void add(ILayer* layer);
        void initLayers(Window* window);
        inline std::vector<ILayer*> list() { return m_Layers; };

    private:
        std::vector<ILayer*> m_Layers;
    };
}
