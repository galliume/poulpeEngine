#pragma once

namespace Rbk
{
	class IRendererAdapter
	{
	public:
		virtual void Init() = 0;
		virtual void Draw() = 0;
		virtual void Destroy() = 0;
	};
}