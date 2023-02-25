#ifndef VKINIT_H
#define VKINIT_H

#include "types.h"

void createInstance(Context* app);
bool checkValidationLayerSupport(void);
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);
bool verifyExtensionSupport(uint32_t extensionCount, VkExtensionProperties* extensions, uint32_t glfwExtensionCount, const char** glfwExtensions);

void setupDebugMessenger(Context* app);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);

void createSurface(Context* app);

void pickPhysicalDevice(Context* app);
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
bool checkDeviceExtensionSupport(VkPhysicalDevice device);
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

void createLogicalDevice(Context* app);
void getFamilyDeviceQueues(VkDeviceQueueCreateInfo* queues, QueueFamilyIndices indices);

void createSwapChain(Context* app);
SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(uint32_t formatCount, VkSurfaceFormatKHR* availableFormats);
VkPresentModeKHR chooseSwapPresentMode(uint32_t presentModeCount, VkPresentModeKHR* availablePresentModes);
VkExtent2D chooseSwapExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR capabilities);
uint32_t clamp(uint32_t n, uint32_t min, uint32_t max);

void createImageViews(Context* context);

void createRenderPass(Context* context);

void createComputeDescriptorSetLayout(Context* context);

void createGraphicsPipeline(Context* context);
uint32_t readFile(const char* filename, char** buffer);
VkShaderModule createShaderModule(VkDevice device, uint8_t* code, uint32_t codeSize);
void getBindingDescriptions(VkVertexInputBindingDescription* bindingDescriptions);
void getAttributeDescriptions(VkVertexInputAttributeDescription* attribute_descriptions);

void createFramebuffers(Context* context);

void createCommandPool(Context* context);

void createVertexBuffer(Context* context);
void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void copyBuffer(Context* context, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createCommandBuffers(Context* context);

void createSyncObjects(Context* context);


void checkErr(VkResult result, char* msg);

#endif
