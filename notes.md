# Notes about my understanding of Vulkan.

### Preparation

glfwCreateWindow

glfwGetWindowSize

glfwGetRequiredInstanceExtensions

vkCreateInstance

CreateDebugUtilsMessengerEXT

vkEnumeratePhysicalDevices

vkGetPhysicalDeviceQueueFamilyProperties

vkCreateDevice

vkCreateWin32SurfaceKHR (glfwCreateWindowSurface)

vkGetPhysicalDeviceSurfaceSupportKHR

vkGetPhysicalDeviceSurfaceFormatsKHR

vkCreateSwapchainKHR

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR, vkGetPhysicalDeviceSurfaceFormatsKHR, vkGetPhysicalDeviceSurfacePresentModesKHR	
	
vkCreateSemaphore (acquire and release)

vkGetDeviceQueue

vkCreateRenderPass

//load shaders

//pipelinecache ?

vkCreatePipelineLayout

vkCreateGraphicsPipelines

vkGetSwapchainImagesKHR

vkCreateBufferView 

	-> used by shader (a.k.a texel view)
	-> maxTexelBufferElements
	
vkCreateImageView (as much as swapChainImages.size()) 

vkCreateFramebuffer (as much as swapChainImages.size())

vkCreateCommandPool

vkAllocateCommandBuffers

### Main loop

while (true) {

	glfwPollEvents
	vkAcquireNextImageKHR
	vkResetCommandPool
	vkBeginCommandBuffer
	vkCmdBeginRenderPass
	vkCmdSetViewport
	vkCmdSetScissor
	vkCmdBindPipeline
	vkCmdDraw / vkCmdDrawIndexed
	vkCmdEndRenderPass
	vkEndCommandBuffer
	vkQueueSubmit
	vkQueuePresentKHR
	vkDeviceWaitIdle
}


### Destroy

glfwDestroyWindow

detroy all elements in reverse order of creation

vkDestroyInstance

###General info

ImageLayout 

	how image data is stored in GPU
	color_attachment_optimal -> fastest on GPU
	shader_read_optimal -> fastest for shader
	present_src -> when present image
	general -> slowest
	depth
	stencil
		
	either use general for simplicity (not recommended) 
	or use the correct layout and switch between states
	
	Image start in undefined
		-> must be transitionned
		
operation to determine by state

	when rendering -> color_attachment_output
	fragment_stage
	shader_read
	
those data are transfered through pipeline barrier

RenderPass to texture

	read texture from frag shader
	
	ROP : render operator processing (blending, color,)
	TEX : texture unit
		L1 cache
		L2 cache
		tile cache ?
	
	when transitionning :
		flush tile cache
		invalidate texture cache
		
	delay fragment work until writes complete
	
	(GPU timeline)
	render to texture    
	------------------->
	-----draw call ------------>
	----->[VS]--------->[FS]------>
	--shadow pass------>----scene pass---->
	
	split barriers ?
