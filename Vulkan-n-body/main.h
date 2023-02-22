#ifndef MAIN_H
#define MAIN_H

#include "types.h"

uint32_t readFile(const char* filename, char** buffer);

void initWindow(Context* app, uint32_t WIN_WIDTH, uint32_t WIN_HEIGHT);
void initVulkan(Context* app);
void mainLoop(Context* app);
void cleanup(Context* app);

void framebufferResizeCallback(Context* app);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

void cleanupSwapChain(Context* app);

#endif

// copyBuffer uses separate command pool for performance
// copyBuffer uses fences instead of vkQueueWaitIdle for performance
// https://vulkan-tutorial.com/en/Vertex_buffers/Staging_buffer
// create custom allocator that splits up a single allocation among many different objects because maxMemoryAllocationCount is limited to 4096