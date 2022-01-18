#pragma once
#include "rebulkpch.h"
#include "imgui.h"
#include "backends/imgui_impl_opengl3_loader.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace Rebulk
{
	class Im
	{
	public:
		static void Init(GLFWwindow* window);
		static void NewFrame();
		static void Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
		template <typename T, typename... Types>
		void static Text(T arg, Types... args) { ImGui::Text(arg, args...); };
		void static EndChild();
		static void End();
		static void EndFrame();
		static void Separator();
		static void BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
		static void Render(GLFWwindow* window);
		static void Destroy();
	};
}