#pragma once
#include "rebulkpch.h"

namespace Rbk
{
	class Im
	{
	public:
		static void Init(GLFWwindow* window, ImGui_ImplVulkan_InitInfo initInfo, VkRenderPass renderPass);
		static void NewFrame();
		static void Begin(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0);
		static void CreateFontsTexture(VkCommandBuffer commandBuffer);
		template <typename T, typename... Types>
		void static Text(T arg, Types... args) { ImGui::Text(arg, args...); };
		void static EndChild();
		static void End();
		static void EndFrame();
		static void Separator();
		static void BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags);
		static void Render(GLFWwindow* window, VkCommandBuffer commandBuffer, VkPipeline pipeline);
		static void Destroy();
	};
}