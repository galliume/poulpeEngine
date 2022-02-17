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

### Memory management

vkAllocateMemory() : contains the memory size and type (used for buffers and images), vkFreeMemory() to free it. Needs to be 
externally synchronized if use in multi threads

vkMapMemory() : map memory between GPU and host (GPU has accces to memory shared by the host)

vkInvalidateMappedMemoryRanges(), vkFlushMappedMemoryRanges() invalidate cache

vkGetBufferMemoryRequirements(), vkGetImageMemoryRequirements() : get memory requirements 

vkBindBufferMemory(), vkBindImageMemory() : bind memory to resource

do not create a memory per object, create a pool of small number of quite large memory allocations, allocate resource at different offset

### Pool and command buffers

vkCreateCommandPool() : pool to handle command buffers

vkAllocateCommandBuffers() : create command buffers, we can record many commands in a commandBuffers.
If multi thread, be sure to not record a command in each thread simultanously. (easiest way of achieving multi thread is to have a command buffers by thread and not sharing them)

Before starting to record command :

vkBeginCommandBuffer() : reset to an initial state

vkEndCommandBuffer() : to end the recording state

vkResetCommandBuffer() : allow to reuse a command buffer instead of destroying it if the same command are recorded over and over

vkResetCommandPool() : same as vkResetCommandBuffer but reset all command buffers in a pool in one shot (nothing is freed, still need to destroy command buffers)

### Submission to the device

vkQueueSubmit : submit one or many command buffers to the device (multiple VkSubmitInfo).
External synchronization is needed with fence / semaphore

vkDeviceWaitIdle() : to wait the end of the execution of all command buffers submitted (not recommanded fully flush work on queue or device, heavyweight operation)

### Synchronizing

vkCmdPipelineBarrier() : create a barrier and give details to the pipeline on when to continue / stop work.
need an Image memory barrier struct

### Swap chain

vkCreateSwapchainKHR() : create a swap chain

### Shaders

vkCreateShaderModule() : load a shader

vkCreateGraphicsPipelines() | vkCreateComputePipelines() : create a pipeline for the shaders (one of heaviest operation)

vkCreatePipelineCache : create cache

vkCmdBindPipeline() : bind pipeline full of shaders to a command buffer

### Descriptor Sets

Describe the resources accessible to the shaders
For one descriptor set, we can bound many resources (textures, sampler, buffers). 
Each descriptor set has a layout describing the order and types of resources
Those layout are grouped into a pipeline layout
a pipeline is created with a pipeline layout

vkCreateDescriptorSetLayout() to create descriptor set layout

vkCreatePipelineLayout() to create pipeline layout

vkUpdateDescriptorSets() bind resources into a descriptor set

### General info

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
