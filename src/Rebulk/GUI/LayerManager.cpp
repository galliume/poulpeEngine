#include "rebulkpch.h"
#include "LayerManager.h"

namespace Rbk
{
    LayerManager::~LayerManager()
    {
        std::cout << "LayerManager deleted" << std::endl;
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
