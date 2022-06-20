#pragma once
#include <memory>

#include "Rebulk/Component/Mesh.h"

namespace Rbk
{
    class IVisitor
    {
    public:
        //@todo make it weak ptr
        virtual void VisitDraw(Mesh* entity) = 0;
        virtual void VisitPrepare(Mesh* entity) = 0;

        virtual ~IVisitor();
    };

    IVisitor::~IVisitor()
    {

    }
}