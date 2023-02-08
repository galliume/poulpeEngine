#include "rebulkpch.hpp"
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

    void LayerManager::InitLayers()
    {
        for (auto layer : m_Layers)
        {
            layer->Init();
        }
    }
}
