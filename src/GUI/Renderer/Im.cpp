#include "Im.h"

namespace Rebulk {

	void Im::Init(GLFWwindow* window, ImGui_ImplVulkan_InitInfo* initInfo, VkRenderPass renderPass)
	{
		const char* glsl_version = "#version 150";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         
		io.ConfigDockingWithShift = false;
		io.ConfigViewportsNoAutoMerge = true;
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

		ImGui::StyleColorsDark();

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_Init(initInfo, renderPass);
	}

	void Im::NewFrame()
	{
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
	}
	
	void Im::CreateFontsTexture(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
	}

	void Im::Begin(const char* name, bool* p_open, ImGuiWindowFlags flags)
	{
		ImGui::Begin(name, p_open, flags);
	}
	
	void Im::End()
	{
		ImGui::End();
	}

	void Im::EndFrame()
	{
		ImGui::EndFrame();
	}

	void Im::EndChild()
	{
		ImGui::EndChild();
	}

	void Im::Separator()
	{
		ImGui::Separator();
	}

	void Im::BeginChild(const char* str_id, const ImVec2& size_arg, bool border, ImGuiWindowFlags extra_flags)
	{
		ImGui::BeginChild(str_id, size_arg, border, extra_flags);
	}

	void Im::Render(GLFWwindow* window, VkCommandBuffer commandBuffer, VkPipeline pipeline)
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Render();
		int displayW, displayH;
		glfwGetFramebufferSize(window, &displayW, &displayH);
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, pipeline);

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void Im::Destroy()
	{
		ImGui_ImplVulkan_DestroyFontUploadObjects();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
}