#pragma once
#include "rebulkpch.h"

namespace Rbk
{
	class IObserver
	{
	public:
		virtual ~IObserver() {};
		//@todo use json
		virtual void Update(std::vector<std::string>& messages) = 0;
	};
}