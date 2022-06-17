#include "rebulkpch.h"
#include "VisitorDrawable.h"

namespace Rbk
{
    void VisitorDrawable::VisitDrawable(Rbk::Drawable* drawable)
    {
        Rbk::Log::GetLogger()->debug("Visiting drawable");
    }
}