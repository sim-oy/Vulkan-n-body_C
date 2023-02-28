#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef NDEBUG
#define ENABLEVALIDATIONLAYERS false
#else
#define ENABLEVALIDATIONLAYERS true
#endif

typedef struct {
    float x, y;
} vec2;

typedef struct {
    float x, y, z;
} vec3;

typedef struct {
    vec2 pos;
    vec2 vel;
    float mss;
    vec3 col;
} Particle;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t formatCount;
    uint32_t presentModeCount;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;
} SwapChainSupportDetails;

typedef struct UniformBufferObject {
    const float deltaTime;
} UniformBufferObject;

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily; // includes ComputeFamily
    bool HasGraphicsFamily;
    uint32_t presentFamily;
    bool HasPresentFamily;
} QueueFamilyIndices;

typedef struct Context {
    GLFWwindow* window;
    const char* WIN_NAME;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    QueueFamilyIndices queueFamilyIndices;
    VkDevice device; // logical device
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapChain;
    VkImage* swapChainImages;
    uint32_t swapChainImageCount;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkImageView* swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer* swapChainFramebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    VkSemaphore* imageAvailableSemaphores;
    VkSemaphore* renderFinishedSemaphores;
    VkFence* inFlightFences;
    const uint32_t MAX_FRAMES_IN_FLIGHT;
    uint32_t currentFrame;
    bool framebufferResized;

    const uint32_t PARTICLE_COUNT;
    
    VkQueue computeQueue;
    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet* computeDescriptorSets;

    VkBuffer* shaderStorageBuffers;
    VkDeviceMemory* shaderStorageBuffersMemory;

    VkBuffer* uniformBuffers;
    VkDeviceMemory* uniformBuffersMemory;
    void** uniformBuffersMapped;

    VkCommandBuffer* computeCommandBuffers;

    VkSemaphore* computeFinishedSemaphores;
    VkFence* computeInFlightFences;
} Context;

#endif
