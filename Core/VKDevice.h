/*
 * Copyright (c) 2021 Valdemar Lindberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _FVK_VK_DEVICE_H_
#define _FVK_VK_DEVICE_H_ 1
#include "VKHelper.h"
#include "VKUtil.h"
#include "VkPhysicalDevice.h"
#include "VulkanCore.h"
#include <fmt/core.h>
#include <optional>
#include <unordered_map>

/**
 * @brief
 *
 */
class FVK_DECL_EXTERN VKDevice {
  public:
	// TODO add support for group device.!
	/**
	 * @brief Construct a new VKDevice object
	 *
	 * @param physicalDevices
	 * @param requested_extensions
	 * @param requiredQueues
	 */
	VKDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physicalDevices,
			 const std::unordered_map<const char *, bool> &requested_extensions = {{"VK_KHR_swapchain", true}},
			 VkQueueFlags requiredQueues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
	// TODO add std::function for override the select GPU.

	VKDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice,
			 const std::unordered_map<const char *, bool> &requested_extensions = {{"VK_KHR_swapchain", true}},
			 VkQueueFlags requiredQueues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
	VKDevice(const VKDevice &) = delete;
	VKDevice(VKDevice &&) = delete;
	~VKDevice();

	/**
	 * @brief
	 *
	 * @return true
	 * @return false
	 */
	bool isGroupDevice() const noexcept { return this->getPhysicalDevices().size() > 0; }
	const std::vector<std::shared_ptr<PhysicalDevice>> &getPhysicalDevices() const noexcept { return this->physicalDevices; }

	unsigned int getNrPhysicalDevices() const noexcept { return this->physicalDevices.size(); }
	const std::shared_ptr<PhysicalDevice> &getPhysicalDevice(unsigned int index) const {
		return this->physicalDevices[index];
	}

	VkDevice getHandle() const noexcept { return this->logicalDevice; }

	VkQueue getDefaultGraphicQueue() const noexcept { return this->graphicsQueue; }
	VkQueue getDefaultPresent() const noexcept { return this->presentQueue; }
	VkQueue getDefaultCompute() const noexcept { return this->computeQueue; }
	VkQueue getDefaultTransfer() const noexcept { return this->transferQueue; }

	uint32_t getDefaultGraphicQueueIndex() const noexcept { return this->graphics_queue_node_index; }
	uint32_t getDefaultComputeQueueIndex() const noexcept { return this->compute_queue_node_index; }
	uint32_t getDefaultTransferQueueIndex() const noexcept { return this->transfer_queue_node_index; }

	/**
	 * @brief
	 *
	 * @param typeFilter
	 * @param properties
	 * @return uint32_t
	 */
	template <size_t n = 0>
	std::optional<uint32_t> findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
		return VKHelper::findMemoryType(physicalDevices[0]->getMemoryProperties(), typeFilter, properties);
	}

	/**
	 * @brief Create a Command Pool object
	 *
	 * @param queue
	 * @param flag
	 * @param pNext
	 * @return VkCommandPool
	 */
	VkCommandPool createCommandPool(uint32_t queue,
									VkCommandPoolCreateFlags flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
									const void *pNext = nullptr) {
		VkCommandPool pool;
		/*  Create command pool.    */
		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = pNext;
		cmdPoolCreateInfo.queueFamilyIndex = queue;
		cmdPoolCreateInfo.flags = flag;

		/*  Create command pool.    */
		VKS_VALIDATE(vkCreateCommandPool(getHandle(), &cmdPoolCreateInfo, nullptr, &pool));

		return pool;
	}

	void submitCommands(VkQueue queue, const std::vector<VkCommandBuffer> &cmd,
						const std::vector<VkSemaphore> &waitSemaphores = {},
						const std::vector<VkSemaphore> &signalSempores = {}, VkFence fence = VK_NULL_HANDLE,
						const std::vector<VkPipelineStageFlags> &waitStages = {
							VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}) {
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		/*	*/
		submitInfo.commandBufferCount = cmd.size();
		submitInfo.pCommandBuffers = cmd.data();

		/*	*/
		submitInfo.signalSemaphoreCount = signalSempores.size();
		submitInfo.pSignalSemaphores = signalSempores.data();

		VKS_VALIDATE(vkQueueSubmit(queue, 1, &submitInfo, fence));
	}

	std::vector<VkCommandBuffer> allocateCommandBuffers(VkCommandPool commandPool, VkCommandBufferLevel level,
														unsigned int nrCmdBuffers = 1, const void *pNext = nullptr) {
		std::vector<VkCommandBuffer> cmdBuffers(nrCmdBuffers);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.pNext = pNext;
		allocInfo.level = level;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = nrCmdBuffers;

		VKS_VALIDATE(vkAllocateCommandBuffers(getHandle(), &allocInfo, cmdBuffers.data()));

		return cmdBuffers;
	}

	std::vector<VkCommandBuffer>
	beginSingleTimeCommands(VkCommandPool commandPool, VkCommandBufferLevel level, unsigned int nrCmdBuffers = 1,
							VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
							VkCommandBufferInheritanceInfo *pInheritInfo = nullptr, const void *pNext = nullptr) {
		std::vector<VkCommandBuffer> cmd = allocateCommandBuffers(commandPool, level, nrCmdBuffers);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = pNext;
		beginInfo.flags = usage;
		beginInfo.pInheritanceInfo = pInheritInfo;

		VKS_VALIDATE(vkBeginCommandBuffer(cmd[0], &beginInfo));

		return cmd;
	}

	void endSingleTimeCommands(VkQueue queue, VkCommandBuffer commandBuffer, VkCommandPool commandPool) {

		VKS_VALIDATE(vkEndCommandBuffer(commandBuffer));

		const std::vector<VkCommandBuffer> cmds = {commandBuffer};
		submitCommands(queue, cmds);
		VKS_VALIDATE(vkQueueWaitIdle(queue));

		vkFreeCommandBuffers(getHandle(), commandPool, cmds.size(), cmds.data());
	}

	/**
	 * @brief
	 *
	 * @param format
	 * @return true
	 * @return false
	 */
	bool isFormatSupported(VkFormat format, VkImageType imageType, VkImageTiling tiling,
						   VkImageUsageFlags usage) const noexcept {
		return this->physicalDevices[0]->isFormatSupported(format, imageType, tiling, usage);
	}

  private:
	uint32_t graphics_queue_node_index;
	uint32_t compute_queue_node_index;
	uint32_t transfer_queue_node_index;
	uint32_t present_queue_node_index;
	uint32_t sparse_queue_node_index;

	std::vector<std::shared_ptr<PhysicalDevice>> physicalDevices;
	VkDevice logicalDevice;

	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkQueue computeQueue;
	VkQueue transferQueue;
	VkQueue sparseQueue;
};

#endif
