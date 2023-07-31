#pragma once
#include "Poulpe/GUI/Layer/ILayer.hpp"

namespace Poulpe
{
    class LayerManager
    {
    public:
        ~LayerManager();
        void Add(ILayer* layer);
        void InitLayers(Window* window);
        inline std::vector<ILayer*> List() { return m_Layers; };

    private:
        std::vector<ILayer*> m_Layers;
    };
}
