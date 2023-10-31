#include "LayerManager.hpp"

namespace Poulpe
{
    LayerManager::~LayerManager()
    {
    }

    void LayerManager::Add(ILayer* layer)
    {
        m_Layers.emplace_back(layer);
    }

    void LayerManager::InitLayers([[maybe_unused]] Window* window)
    {
        /*for (auto layer : m_Layers)
        {
            layer->Init(window);
        }*/
    }
}
