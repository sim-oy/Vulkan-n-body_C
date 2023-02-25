#include "vkinit.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint32_t validationLayerCount = 1;
const char* validationLayers[] = { "VK_LAYER_KHRONOS_validation" };
const uint32_t deviceExtensionsCount = 1;
const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

void createInstance(Context* context) {
    if (ENABLEVALIDATIONLAYERS && !checkValidationLayerSupport()) {
        printf("validation layers requested, but not available!\n");
        exit(1);
    }

    VkApplicationInfo contextInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = context->WIN_NAME,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
        .pNext = NULL
    };

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    const char** glfwExtensionsWithDebug = malloc(sizeof(const char*) * (glfwExtensionCount + 1));

    for (int i = 0; i < glfwExtensionCount; i++) {
        glfwExtensionsWithDebug[i] = glfwExtensions[i];
    }
    if (ENABLEVALIDATIONLAYERS) {
        glfwExtensionsWithDebug[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = { 0 };
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &contextInfo
    };
    if (ENABLEVALIDATIONLAYERS) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.enabledExtensionCount = glfwExtensionCount + 1;
        createInfo.ppEnabledExtensionNames = glfwExtensionsWithDebug;

        populateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.pNext = NULL;
    }

    VkResult result = vkCreateInstance(&createInfo, NULL, &context->instance);
    checkErr(result, "failed to create vk instance!");

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties* extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);

    if (!verifyExtensionSupport(extensionCount, extensions, glfwExtensionCount, glfwExtensions)) {
        printf("Not all extensions supported!\n");
        free(extensions);
        exit(1);
    }
    free(extensions);
    free(glfwExtensionsWithDebug);
}

bool checkValidationLayerSupport(void) {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (int i = 0; i < validationLayerCount; i++) {
        bool layerFound = false;
        for (int j = 0; j < layerCount; j++) {
            if (strcmp(availableLayers[j].layerName, validationLayers[i]) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            free(availableLayers);
            return false;
        }
    }

    free(availableLayers);
    return true;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
    createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = debugCallback;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    printf("validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

bool verifyExtensionSupport(uint32_t extensionCount, VkExtensionProperties* extensions, uint32_t glfwExtensionCount, const char** glfwExtensions) {
    for (int i = 0; i < glfwExtensionCount; i++) {
        bool extensionFound = false;
        for (int j = 0; j < extensionCount; j++) {
            if (strcmp(extensions[j].extensionName, glfwExtensions[i]) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            return false;
        }
    }
    return true;
}

void setupDebugMessenger(Context* context) {
    if (!ENABLEVALIDATIONLAYERS) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo = { 0 };
    populateDebugMessengerCreateInfo(&createInfo);
    //createInfo.pUserData = NULL;

    VkResult result = CreateDebugUtilsMessengerEXT(context->instance, &createInfo, NULL, &context->debugMessenger);
    checkErr(result, "Failed to setup debug messenger!");
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != NULL) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void createSurface(Context* context) {
    VkResult result = glfwCreateWindowSurface(context->instance, context->window, NULL, &context->surface);
    checkErr(result, "failed to create window surface!");
}

void pickPhysicalDevice(Context* context) {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(context->instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        printf("Failed to find GPUs with Vulkan support\n!");
        exit(1);
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * deviceCount);
    VkResult result = vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices);
    checkErr(result, "Failed to enumerate physical devices!");

    for (int i = 0; i < deviceCount; i++) {
        if (isDeviceSuitable(devices[i], context->surface)) {
            context->physicalDevice = devices[i];
            break;
        }
    }

    if (context->physicalDevice == NULL) {
        printf("Failed to find a suitable GPU!\n");
        exit(1);
    }
    else {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(context->physicalDevice, &deviceProperties);
        printf("Selected device: %s\n", deviceProperties.deviceName);
    }

    context->queueFamilyIndices = findQueueFamilies(context->physicalDevice, context->surface);
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = findQueueFamilies(device, surface);
    if (!(indices.HasGraphicsFamily && indices.HasPresentFamily)) {
        printf("Queuefalmily not supported!\n");
        return false;
    }
    bool extensionsSupported = checkDeviceExtensionSupport(device);
    if (!extensionsSupported) {
        printf("Extensions not supported!\n");
        return false;
    }
    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device, surface);
    swapChainAdequate = (swapChainSupport.formatCount == 0) || (swapChainSupport.presentModeCount == 0);
    if (swapChainAdequate) {
        printf("Swapchain not adequeate!\n");
        return false;
    }

    return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

    VkExtensionProperties* availableExtensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extensionCount);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

    for (int i = 0; i < deviceExtensionsCount; i++) {
        bool extensionFound = false;
        for (int j = 0; j < extensionCount; j++) {
            if (strcmp(deviceExtensions[i], availableExtensions[j].extensionName) == 0) {
                extensionFound = true;
                break;
            }
        }
        if (!extensionFound) {
            free(availableExtensions);
            return false;
        }
    }
    free(availableExtensions);
    return true;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = { 0 };

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    VkQueueFamilyProperties* queueFamilyProperties = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyProperties);


    for (int i = 0; i < queueFamilyCount; i++) {
        if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.graphicsFamily = i;
            indices.HasGraphicsFamily = true;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
            indices.HasPresentFamily = true;
        }
        if (indices.HasGraphicsFamily && indices.HasPresentFamily) {
            break;
        }
    }
    free(queueFamilyProperties);
    return indices;
}

void createLogicalDevice(Context* context) {
    QueueFamilyIndices indices = findQueueFamilies(context->physicalDevice, context->surface);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(context->physicalDevice, &deviceFeatures);

    VkDeviceQueueCreateInfo queues[2];
    getFamilyDeviceQueues(queues, indices);

    VkDeviceCreateInfo createInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      //.pQueueCreateInfos = &queueCreateInfo,
      .pQueueCreateInfos = queues,
      .queueCreateInfoCount = 1,
      .pEnabledFeatures = &deviceFeatures,
      .enabledExtensionCount = deviceExtensionsCount,
      .ppEnabledExtensionNames = deviceExtensions
    };

    if (ENABLEVALIDATIONLAYERS) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateDevice(context->physicalDevice, &createInfo, NULL, &context->device);
    checkErr(result, "failed to create logical device!");

    vkGetDeviceQueue(context->device, context->queueFamilyIndices.graphicsFamily, 0, &context->graphicsQueue);
    vkGetDeviceQueue(context->device, context->queueFamilyIndices.graphicsFamily, 0, &context->computeQueue);
    vkGetDeviceQueue(context->device, context->queueFamilyIndices.presentFamily, 0, &context->presentQueue);
}

void getFamilyDeviceQueues(VkDeviceQueueCreateInfo* queues, QueueFamilyIndices indices) {

    float QueuePriority = 1.0f;
    // GraphicsQueue
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.graphicsFamily,
      .queueCount = 1
    };

    graphicsQueueCreateInfo.pQueuePriorities = &QueuePriority;
    queues[0] = graphicsQueueCreateInfo;

    // PresentQueue
    VkDeviceQueueCreateInfo presentQueueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = indices.presentFamily,
      .queueCount = 1
    };

    presentQueueCreateInfo.pQueuePriorities = &QueuePriority;
    queues[1] = presentQueueCreateInfo;
}

void createSwapChain(Context* context) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(context->physicalDevice, context->surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formatCount, swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModeCount, swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(context->window, swapChainSupport.capabilities);

    free(swapChainSupport.formats);
    free(swapChainSupport.presentModes);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    QueueFamilyIndices indices = findQueueFamilies(context->physicalDevice, context->surface);
    uint32_t queueFamilyIndices[2] = { context->queueFamilyIndices.graphicsFamily, context->queueFamilyIndices.presentFamily };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(context->device, &createInfo, NULL, &context->swapChain);
    checkErr(result, "failed to create swap chain!");

    vkGetSwapchainImagesKHR(context->device, context->swapChain, &imageCount, NULL);
    context->swapChainImageCount = imageCount;
    context->swapChainImages = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(context->device, context->swapChain, &imageCount, context->swapChainImages);

    context->swapChainImageFormat = surfaceFormat.format;
    context->swapChainExtent = extent;
}

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapChainSupportDetails details = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, NULL);
    details.formatCount = formatCount;
    if (formatCount != 0) {
        VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
        details.formats = formats;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, NULL);
    details.presentModeCount = presentModeCount;
    if (presentModeCount != 0) {
        VkPresentModeKHR* presentModes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * presentModeCount);
        details.presentModes = presentModes;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(uint32_t formatCount, VkSurfaceFormatKHR* availableFormats) {
    for (int i = 0; i < formatCount; i++) {
        if (availableFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB && availableFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormats[i];
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(uint32_t presentModeCount, VkPresentModeKHR* availablePresentModes) {
    for (int i = 0; i < presentModeCount; i++) {
        if (availablePresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentModes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(GLFWwindow* window, VkSurfaceCapabilitiesKHR capabilities) {
    if (capabilities.currentExtent.width != UINT_MAX) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = { width, height };

        actualExtent.width = clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

uint32_t clamp(uint32_t n, uint32_t min, uint32_t max) {
    if (n < min) return min;
    if (n > max) return max;
    return n;
}

void createImageViews(Context* context) {
    context->swapChainImageViews = (VkImageView*)malloc(sizeof(VkImageView*) * context->swapChainImageCount);
    for (int i = 0; i < context->swapChainImageCount; i++) {
        VkImageViewCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = context->swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = context->swapChainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        vkCreateImageView(context->device, &createInfo, NULL, &context->swapChainImageViews[i]);
    }
}

void createRenderPass(Context* context) {
    VkAttachmentDescription colorAttachment = {
        .format = context->swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };


    VkResult result = vkCreateRenderPass(context->device, &renderPassInfo, NULL, &context->renderPass);
    checkErr(result, "failed to create render pass!");
}

void createComputeDescriptorSetLayout(Context* context) {
    VkDescriptorSetLayoutBinding layoutBindings[3] = { 0 };
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].pImmutableSamplers = NULL;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].pImmutableSamplers = NULL;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].pImmutableSamplers = NULL;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings = layoutBindings
    };
    
    VkResult result = vkCreateDescriptorSetLayout(context->device, &layoutInfo, NULL, &context->computeDescriptorSetLayout);
    checkErr(result, "failed to create compute descriptor set layout!");
}

void createGraphicsPipeline(Context* context) {
    char* vertShaderCode = NULL;
    char* fragShaderCode = NULL;
    uint32_t vertShaderCodeSize = (uint32_t)readFile("shaders/vert.spv", &vertShaderCode);
    uint32_t fragShaderCodeSize = (uint32_t)readFile("shaders/frag.spv", &fragShaderCode);

    VkShaderModule vertShaderModule = createShaderModule(context->device, vertShaderCode, vertShaderCodeSize);
    VkShaderModule fragShaderModule = createShaderModule(context->device, fragShaderCode, fragShaderCodeSize);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = NULL
    };
    VkPipelineShaderStageCreateInfo shaderStages[2] = { vertShaderStageInfo, fragShaderStageInfo };

    VkVertexInputBindingDescription bindingDescriptions[1];
    getBindingDescriptions(bindingDescriptions);
    VkVertexInputAttributeDescription attributeDescriptions[2];
    getAttributeDescriptions(attributeDescriptions);
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = bindingDescriptions,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attributeDescriptions
    };

    uint32_t dynamicStateSize = 2;
    VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = dynamicStateSize,
        .pDynamicStates = dynamicStates
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)context->swapChainExtent.width,
        .height = (float)context->swapChainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = context->swapChainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor

    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f, // Optional
        .depthBiasClamp = 0.0f, // Optional
        .depthBiasSlopeFactor = 0.0f // Optional
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .minSampleShading = 1.0f, // Optional
        .pSampleMask = NULL, // Optional
        .alphaToCoverageEnable = VK_FALSE, // Optional
        .alphaToOneEnable = VK_FALSE // Optional
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .colorBlendOp = VK_BLEND_OP_ADD, // Optional
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
        .alphaBlendOp = VK_BLEND_OP_ADD // Optional
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY, // Optional
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants[0] = 0.0f, // Optional
        .blendConstants[1] = 0.0f, // Optional
        .blendConstants[2] = 0.0f, // Optional
        .blendConstants[3] = 0.0f // Optional
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0, // Optional
        .pSetLayouts = NULL, // Optional
        .pushConstantRangeCount = 0, // Optional
        .pPushConstantRanges = NULL // Optional
    };

    VkResult result = vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &context->pipelineLayout);
    checkErr(result, "failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = NULL, // Optional
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = context->pipelineLayout,
        .renderPass = context->renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE, // Optional
        .basePipelineIndex = -1, // Optional
    };

    result = vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &context->graphicsPipeline);
    checkErr(result, "failed to create graphics pipeline!");

    vkDestroyShaderModule(context->device, fragShaderModule, NULL);
    vkDestroyShaderModule(context->device, vertShaderModule, NULL);
    free(vertShaderCode);
    free(fragShaderCode);
}

uint32_t readFile(const char* filename, char** buffer) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Failed to open file!");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (length <= 0) {
        printf("Failed to get length of file!");
        fclose(file);
        exit(1);
    }
    *buffer = (char*)malloc(length * sizeof(char));
    if (*buffer == NULL) {
        printf("Failed to allocate buffer!");
        fclose(file);
        exit(1);
    }
    fread(*buffer, sizeof(char), length, file);
    fclose(file);
    return length;
}

VkShaderModule createShaderModule(VkDevice device, uint8_t* code, uint32_t codeSize) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = codeSize,
        .pCode = (uint32_t*)code
    };

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);
    checkErr(result, "failed to create shader module!");

    return shaderModule;
}

void getBindingDescriptions(VkVertexInputBindingDescription* bindingDescriptions) {
    bindingDescriptions[0] = (VkVertexInputBindingDescription){ 0 };
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void getAttributeDescriptions(VkVertexInputAttributeDescription* attributeDescriptions) {
    attributeDescriptions[0] = (VkVertexInputAttributeDescription){ 0 };
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1] = (VkVertexInputAttributeDescription){ 0 };
    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
}

void createFramebuffers(Context* context) {
    context->swapChainFramebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * context->swapChainImageCount);

    for (uint32_t i = 0; i < context->swapChainImageCount; i++) {
        VkImageView attachments[1] = { context->swapChainImageViews[i] };

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = context->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = context->swapChainExtent.width,
            .height = context->swapChainExtent.height,
            .layers = 1
        };
        VkResult result = vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &context->swapChainFramebuffers[i]);
        checkErr(result, "failed to create render pass!");
    }
}

void createCommandPool(Context* context) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(context->physicalDevice, context->surface);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily
    };
    VkResult result = vkCreateCommandPool(context->device, &poolInfo, NULL, &context->commandPool);
    checkErr(result, "failed to create command pool!");
}

void createVertexBuffer(Context* context) {

    VkDeviceSize bufferSize = sizeof(context->vertices.verts[0]) * context->vertices.size;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(context->physicalDevice,
        context->device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer,
        &stagingBufferMemory);

    void* data;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, context->vertices.verts, (size_t)bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    createBuffer(context->physicalDevice,
        context->device, bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &context->vertexBuffer,
        &context->vertexBufferMemory);

    copyBuffer(context, context->commandPool, stagingBuffer, context->vertexBuffer, bufferSize);

    vkDestroyBuffer(context->device, stagingBuffer, NULL);
    vkFreeMemory(context->device, stagingBufferMemory, NULL);
}

void createBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory) {

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VkResult result = vkCreateBuffer(device, &bufferInfo, NULL, buffer);
    checkErr(result, "failed to create buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties)
    };

    result = vkAllocateMemory(device, &allocInfo, NULL, bufferMemory);
    checkErr(result, "failed to allocate buffer memory!");

    result = vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
    checkErr(result, "failed to bind buffer memory!");
}

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type!\n");
    exit(1);
}

void copyBuffer(Context* context, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context->device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0, // Optional
        .dstOffset = 0, // Optional
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context->graphicsQueue);

    vkFreeCommandBuffers(context->device, commandPool, 1, &commandBuffer);
}

void createCommandBuffers(Context* context) {
    context->commandBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * context->MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = context->MAX_FRAMES_IN_FLIGHT
    };
    VkResult result = vkAllocateCommandBuffers(context->device, &allocInfo, context->commandBuffers);
    checkErr(result, "failed to allocate command buffers!");
};

void createSyncObjects(Context* context) {
    context->imageAvailableSemaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * context->MAX_FRAMES_IN_FLIGHT);
    context->renderFinishedSemaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * context->MAX_FRAMES_IN_FLIGHT);
    context->inFlightFences = (VkFence*)malloc(sizeof(VkFence) * context->MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (uint32_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreInfo, NULL, &context->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, NULL, &context->inFlightFences[i]) != VK_SUCCESS) {
            printf("failed to create semaphores or fence!\n");
            exit(1);
        }
    }
}


void createShaderStorageBuffers(Context* context) {

    // Initial particle positions on a circle
    Particle* particles = (Particle*)malloc(context->PARTICLE_COUNT * sizeof(Particle));

    for (int i = 0; i < context->PARTICLE_COUNT; i++) {
        particles[i].pos.x = (float)rand() / (float)RAND_MAX;
        particles[i].pos.y = (float)rand() / (float)RAND_MAX;
        particles[i].vel.x = (float)rand() / (float)RAND_MAX;
        particles[i].vel.y = (float)rand() / (float)RAND_MAX;
        particles[i].mss = (float)rand() / (float)RAND_MAX;
        particles[i].col.x = 1.0f;
        particles[i].col.y = 1.0f;
        particles[i].col.z = 1.0f;
    }

    VkDeviceSize bufferSize = sizeof(Particle) * context->PARTICLE_COUNT;

    // Create a staging buffer used to upload data to the gpu
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(context->physicalDevice, 
        context->device, 
        bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        &stagingBuffer, 
        &stagingBufferMemory);

    void* data;
    vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles, (size_t)bufferSize);
    vkUnmapMemory(context->device, stagingBufferMemory);

    context->shaderStorageBuffers = (VkBuffer*)malloc(sizeof(VkBuffer) * context->MAX_FRAMES_IN_FLIGHT);
    context->shaderStorageBuffersMemory = (VkDeviceMemory*)malloc(sizeof(VkDeviceMemory) * context->MAX_FRAMES_IN_FLIGHT);

    // Copy initial particle data to all storage buffers
    for (size_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(context->physicalDevice,
            context->device, 
            bufferSize,
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            context->shaderStorageBuffers[i], 
            context->shaderStorageBuffersMemory[i]);
        copyBuffer(context, context->commandPool, stagingBuffer, context->shaderStorageBuffers[i], bufferSize);
    }

    vkDestroyBuffer(context->device, stagingBuffer, NULL);
    vkFreeMemory(context->device, stagingBufferMemory, NULL);
    free(particles);
}

void createComputePipeline(Context* context) {

    char* compShaderCode = NULL;
    uint32_t compShaderCodeSize = (uint32_t)readFile("shaders/comp.spv", &compShaderCode);

    VkShaderModule compShaderModule = createShaderModule(context->device, compShaderCode, compShaderCodeSize);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = compShaderModule,
        .pName = "main"
    };
    

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &context->computeDescriptorSetLayout
    };
    
    VkResult result = vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL, &context->computePipelineLayout);
    checkErr(result, "failed to create compute pipeline layout!");

    VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .layout = context->computePipelineLayout,
        .stage = computeShaderStageInfo
    };

    result = vkCreateComputePipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &context->computePipeline);
    checkErr(result, "failed to create compute pipeline!");

    vkDestroyShaderModule(context->device, compShaderModule, NULL);
}

void createComputeDescriptorSets(Context* context) {
    VkDescriptorSetLayout* layouts = (VkDescriptorSetLayout*)malloc(sizeof(VkDescriptorSetLayout) * context->MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = context->computeDescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = context->descriptorPool, // do createDescriptorPools!!
        .descriptorSetCount = context->MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts
    };
    
    context->computeDescriptorSets = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet) * context->MAX_FRAMES_IN_FLIGHT);
    VkResult result = vkAllocateDescriptorSets(context->device, &allocInfo, context->computeDescriptorSets);
    checkErr(result, "failed to allocate descriptor sets!");

    for (size_t i = 0; i < context->MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo uniformBufferInfo = {
            .buffer = uniformBuffers[i], // do createUniformBuffers!!
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        VkWriteDescriptorSet descriptorWrites[3] = { 0 };
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = context->computeDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &uniformBufferInfo;

        VkDescriptorBufferInfo storageBufferInfoLastFrame = {
            .buffer = context->shaderStorageBuffers[(i - 1) % context->MAX_FRAMES_IN_FLIGHT],
            .offset = 0,
            .range = sizeof(Particle) * context->PARTICLE_COUNT
        };

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = context->computeDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &storageBufferInfoLastFrame;

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame = {
            .buffer = context->shaderStorageBuffers[i],
            .offset = 0,
            .range = sizeof(Particle) * context->PARTICLE_COUNT
        };
        
        descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = context->computeDescriptorSets[i];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pBufferInfo = &storageBufferInfoCurrentFrame;

        vkUpdateDescriptorSets(context->device, 3, descriptorWrites, 0, NULL);
    }
}




// MOVE !!!!!
void checkErr(VkResult result, char* msg) {
    if (result != VK_SUCCESS) {
        printf("%s\n", msg);
        exit(1);
    }
}