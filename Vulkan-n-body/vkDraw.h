#ifndef VKDRAW_H
#define VKDRAW_H

#include "types.h"

void recreateSwapChain(Context* app);

void recordCommandBuffer(Context* app, VkCommandBuffer commandBuffer, uint32_t imageIndex);

void drawFrame(Context* app);

void cleanupSwapChain(Context* app);

#endif