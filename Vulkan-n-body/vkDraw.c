#include "vkDraw.h"
#include "vkinit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void recordCommandBuffer(Context* context, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = NULL // Optional
    };
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    checkErr(result, "failed to begin recording command buffer!");

    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->renderPass,
        .framebuffer = context->swapChainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = context->swapChainExtent,
        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphicsPipeline);

        VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = (float)(context->swapChainExtent.width),
            .height = (float)(context->swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = context->swapChainExtent
        };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &context->shaderStorageBuffers[context->currentFrame], offsets);

        // Draw command
        vkCmdDraw(commandBuffer, context->PARTICLE_COUNT, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);
    result = vkEndCommandBuffer(commandBuffer);
    checkErr(result, "failed to record command buffer!");
}

void drawFrame(Context* context) {
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO
    };

    // Compute submission
    vkWaitForFences(context->device, 1, &context->computeInFlightFences[context->currentFrame], VK_TRUE, UINT64_MAX);

    updateUniformBuffer(context->timeStep, context->uniformBuffersMapped, context->currentFrame);

    vkResetFences(context->device, 1, &context->computeInFlightFences[context->currentFrame]);

    vkResetCommandBuffer(context->computeCommandBuffers[context->currentFrame], 0);
    recordComputeCommandBuffer(context, context->computeCommandBuffers[context->currentFrame]);

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &context->computeCommandBuffers[context->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &context->computeFinishedSemaphores[context->currentFrame];

    VkResult result = vkQueueSubmit(context->computeQueue, 1, &submitInfo, context->computeInFlightFences[context->currentFrame]);
    checkErr(result, "failed to submit compute command buffer!");


    // Graphics submission
    vkWaitForFences(context->device, 1, &context->inFlightFences[context->currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    result = vkAcquireNextImageKHR(context->device, context->swapChain, UINT64_MAX, context->imageAvailableSemaphores[context->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain(context);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        printf("failed to acquire swap chain image!");
        exit(1);
    }
    vkResetFences(context->device, 1, &context->inFlightFences[context->currentFrame]);

    vkResetCommandBuffer(context->commandBuffers[context->currentFrame], 0);
    recordCommandBuffer(context, context->commandBuffers[context->currentFrame], imageIndex);

    VkSemaphore waitSemaphores[2] = { context->computeFinishedSemaphores[context->currentFrame], context->imageAvailableSemaphores[context->currentFrame] };
    VkPipelineStageFlags waitStages[2] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo2 = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 2,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &context->commandBuffers[context->currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &context->renderFinishedSemaphores[context->currentFrame]
    };
    result = vkQueueSubmit(context->graphicsQueue, 1, &submitInfo2, context->inFlightFences[context->currentFrame]);
    checkErr(result, "failed to submit draw command buffer!");

    VkSwapchainKHR swapChains[] = { context->swapChain };
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &context->renderFinishedSemaphores[context->currentFrame],
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = NULL // Optional
    };

    result = vkQueuePresentKHR(context->presentQueue, &presentInfo);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || context->framebufferResized) {
        context->framebufferResized = false;
        recreateSwapChain(context);
    }
    else if (result != VK_SUCCESS) {
        printf("failed to present swap chain image!");
        exit(1);
    }

    context->currentFrame = (context->currentFrame + 1) % context->MAX_FRAMES_IN_FLIGHT;
}

void updateUniformBuffer(float timeStep, void** uniformBuffersMapped, uint32_t currentImage) {
    UniformBufferObject ubo = {
        .deltaTime = timeStep
    };

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void recreateSwapChain(Context* context) {

    int width = 0, height = 0;
    glfwGetFramebufferSize(context->window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(context->window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(context->device);

    cleanupSwapChain(context);

    createSwapChain(context);
    createImageViews(context);
    createFramebuffers(context);
}

void cleanupSwapChain(Context* context) {
    for (int i = 0; i < context->swapChainImageCount; i++) {
        vkDestroyFramebuffer(context->device, context->swapChainFramebuffers[i], NULL);
    }
    for (int i = 0; i < context->swapChainImageCount; i++) {
        vkDestroyImageView(context->device, context->swapChainImageViews[i], NULL);
    }
    vkDestroySwapchainKHR(context->device, context->swapChain, NULL);
}

void recordComputeCommandBuffer(Context* context, VkCommandBuffer commandBuffer) {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };

    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    checkErr(result, "failed to begin recording compute command buffer!");

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->computePipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->computePipelineLayout, 0, 1, &context->computeDescriptorSets[context->currentFrame], 0, NULL);

    vkCmdDispatch(commandBuffer, context->PARTICLE_COUNT / 256, 1, 1);

    result = vkEndCommandBuffer(commandBuffer);
    checkErr(result, "failed to record compute command buffer!");
}