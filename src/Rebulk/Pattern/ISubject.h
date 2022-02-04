#pragma once
#include "Rebulk/Pattern/IObserver.h"

namespace Rbk
{
	class ISubject
	{
	public:
		virtual ~ISubject() {};
		virtual void Attach(Rbk::IObserver* observer) = 0;
		virtual void Detach(Rbk::IObserver* observer) = 0;
		virtual void Notify() = 0;
	};
}