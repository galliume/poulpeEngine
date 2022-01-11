#pragma once
#include <iostream>
#include <list>
#include <string>

namespace Rebulk
{
	class IObserver
	{
	public:
		virtual ~IObserver() {};
		//@todo use json
		virtual void Update(std::string& title, std::vector<std::string>& messages) = 0;
	};
}