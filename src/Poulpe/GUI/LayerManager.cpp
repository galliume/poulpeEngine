#include "LayerManager.hpp"

namespace Poulpe
{
    LayerManager::~LayerManager()
    {
    }

    void LayerManager::add(ILayer* layer)
    {
        m_Layers.emplace_back(layer);
    }

    void LayerManager::initLayers([[maybe_unused]] Window* window)
    {
        /*for (auto layer : m_Layers)
        {
            layer->Init(window);
        }*/
    }
}
