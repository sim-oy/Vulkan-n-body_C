#include "main.h"
#include "vkinit.h"
#include "vkDraw.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FRAMES_PER_PRINT 3000

int main(void) {
    Context context = {
        .WIN_NAME = "Window 1",
        .MAX_FRAMES_IN_FLIGHT = 2,
        .currentFrame = 0,
        .framebufferResized = false,
        .PARTICLE_COUNT = 256 * 1,
        .timeStep = 0.001f
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
    bool printFrameTime = false;
    int frames = 0;
    double times[FRAMES_PER_PRINT] = { 0 };
    clock_t oa_tim_strt = 0, oa_tim_end = 0;
    while (!glfwWindowShouldClose(context->window)) {
        oa_tim_strt = clock();

        glfwPollEvents();
        drawFrame(context);

        oa_tim_end = clock();
        double elapsedTime_s = ((double)(oa_tim_end - oa_tim_strt)) / CLOCKS_PER_SEC;
        times[frames] = elapsedTime_s;
        if (frames >= FRAMES_PER_PRINT - 1 && printFrameTime) {
            double avg_elapsedTime_s = DoubleArraySum(times, FRAMES_PER_PRINT) / (double)FRAMES_PER_PRINT;
            printf("time: ms %d\t fps: %.1lf\n", (int)(avg_elapsedTime_s * 1000), 1.0 / avg_elapsedTime_s);
            frames = 0;
        }
        else if (printFrameTime) {
            frames++;
        }
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

double DoubleArraySum(double* array, int len) {
    double sum = 0;
    for (int i = 0; i < len; i++) {
        sum += array[i];
    }
    return sum;
}