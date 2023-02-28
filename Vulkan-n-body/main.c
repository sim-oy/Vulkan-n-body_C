#include "main.h"
#include "vkinit.h"
#include "vkDraw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    Context context = {
        .WIN_NAME = "Window 1",
        .MAX_FRAMES_IN_FLIGHT = 2,
        .currentFrame = 0,
        .framebufferResized = false,
        .PARTICLE_COUNT = 256
    };
    uint32_t WIN_WIDTH = 800;
    uint32_t WIN_HEIGHT = 600;

    srand(0);

    initWindow(&context, WIN_WIDTH, WIN_HEIGHT);
    initVulkan(&context);
    mainLoop(&context);
    cleanup(&context);

    return 0;
}


void initWindow(Context* context, uint32_t WIN_WIDTH, uint32_t WIN_HEIGHT) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    context->window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, context->WIN_NAME, NULL, NULL);
    glfwSetFramebufferSizeCallback(context->window, framebufferResizeCallback);
}
void framebufferResizeCallback(Context* context) {
    context->framebufferResized = true;
}

void initVulkan(Context* context) {
    createInstance(context);
    setupDebugMessenger(context);
    createSurface(context);
    pickPhysicalDevice(context);
    createLogicalDevice(context);
    createSwapChain(context);
    createImageViews(context);
    createRenderPass(context);
    createComputeDescriptorSetLayout(context);
    createGraphicsPipeline(context);
    createComputePipeline(context);
    createFramebuffers(context);
    createCommandPool(context);
    createShaderStorageBuffers(context);
    createUniformBuffers(context);
    createDescriptorPool(context);
    createComputeDescriptorSets(context);
    createCommandBuffers(context);
    createComputeCommandBuffers(context);
    createSyncObjects(context);
}

void mainLoop(Context* context) {
    while (!glfwWindowShouldClose(context->window)) {
        glfwPollEvents();
        drawFrame(context);
    }
    vkDeviceWaitIdle(context->device);
}

void cleanup(Context* context) {
    cleanupSwapChain(context);

    vkDestroyPipeline(context->device, context->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->pipelineLayout, NULL);

    vkDestroyPipeline(context->device, context->computePipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->computePipelineLayout, NULL);

    vkDestroyRenderPass(context->device, context->renderPass, NULL);

    for (size_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(context->device, context->uniformBuffers[i], NULL);
        vkFreeMemory(context->device, context->uniformBuffersMemory[i], NULL);
    }

    vkDestroyDescriptorPool(context->device, context->descriptorPool, NULL);

    vkDestroyDescriptorSetLayout(context->device, context->computeDescriptorSetLayout, NULL);

    for (size_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(context->device, context->shaderStorageBuffers[i], NULL);
        vkFreeMemory(context->device, context->shaderStorageBuffersMemory[i], NULL);
    }

    for (uint32_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->device, context->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(context->device, context->renderFinishedSemaphores[i], NULL);
        vkDestroySemaphore(context->device, context->computeFinishedSemaphores[i], NULL);
        vkDestroyFence(context->device, context->inFlightFences[i], NULL);
        vkDestroyFence(context->device, context->computeInFlightFences[i], NULL);
    }

    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyDevice(context->device, NULL);

    if (ENABLEVALIDATIONLAYERS) {
        DestroyDebugUtilsMessengerEXT(context->instance, context->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(context->instance, context->surface, NULL);
    vkDestroyInstance(context->instance, NULL);

    glfwDestroyWindow(context->window);
    glfwTerminate();

    free(context->swapChainImages);
    free(context->commandBuffers);
    free(context->imageAvailableSemaphores);
    free(context->renderFinishedSemaphores);
    free(context->computeFinishedSemaphores);
    free(context->inFlightFences);
    free(context->computeInFlightFences);
    free(context->computeDescriptorSets);
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}
