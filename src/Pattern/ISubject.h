#pragma once
#include "Pattern\IObserver.h"

namespace Rebulk
{
	class ISubject
	{
	public:
		virtual ~ISubject() {};
		virtual void Attach(Rebulk::IObserver* observer) = 0;
		virtual void Detach(Rebulk::IObserver* observer) = 0;
		virtual void Notify() = 0;
	};
}