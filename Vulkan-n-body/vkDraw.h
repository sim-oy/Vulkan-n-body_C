#ifndef VKDRAW_H
#define VKDRAW_H

#include "types.h"

void recordCommandBuffer(Context* app, VkCommandBuffer commandBuffer, uint32_t imageIndex);

void drawFrame(Context* app);
void updateUniformBuffer(void** uniformBuffersMapped, uint32_t currentImage);

void cleanupSwapChain(Context* app);

void recreateSwapChain(Context* app);

void recordComputeCommandBuffer(Context* context, VkCommandBuffer commandBuffer);

#endif