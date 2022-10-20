#pragma once
#include "Rebulk/GUI/Layer/ILayer.h"

namespace Rbk
{
    class LayerManager
    {
    public:
        ~LayerManager();
        void Add(ILayer* layer);
        void InitLayers();
        inline std::vector<ILayer*> List() { return m_Layers; };

    private:
        std::vector<ILayer*> m_Layers;
    };
}
