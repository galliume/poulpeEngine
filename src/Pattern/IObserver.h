#pragma once
#include <iostream>
#include <list>
#include <string>

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