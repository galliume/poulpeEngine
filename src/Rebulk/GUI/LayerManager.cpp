#include "LayerManager.hpp"

namespace Rbk
{
    LayerManager::~LayerManager()
    {
    }

    void LayerManager::Add(ILayer* layer)
    {
        m_Layers.emplace_back(layer);
    }

    void LayerManager::InitLayers(Window* window)
    {
        for (auto layer : m_Layers)
        {
            layer->Init(window);
        }
    }
}
