#include "rebulkpch.h"
#include "Drawable.h"

namespace Rbk
{
    void Drawable::Accept(std::weak_ptr<IVisitor> visitor)
    {
        visitor.lock()->VisitDrawable(this);
    };
}