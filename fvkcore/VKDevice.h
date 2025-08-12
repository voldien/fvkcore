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
#pragma once
#include "VKHelper.h"
#include "VKUtil.h"
#include "VkPhysicalDevice.h"
#include "VulkanCore.h"
#include "vulkan/vulkan_core.h"
#include <cstdint>
#include <fmt/core.h>
#include <optional>
#include <unordered_map>
#include <vector>

namespace fvkcore {

	/**
	 * @brief
	 *
	 */
	class FVK_DECL_EXTERN VKDevice {
	  public:
		VKDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physicalDevices,
				 const std::unordered_map<const char *, bool> &requested_extensions = {{"VK_KHR_swapchain", true}},
				 const VkQueueFlags requiredQueues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
				 const void *pNext = nullptr);

		VKDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice,
				 const std::unordered_map<const char *, bool> &requested_extensions = {{"VK_KHR_swapchain", true}},
				const  VkQueueFlags requiredQueues = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
				 const void *pNext = nullptr);

		VKDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physicalDevices,
				 const std::unordered_map<const char *, bool> &requested_extensions,
				 const std::vector<VkDeviceQueueCreateInfo> &queues, const void *pNext = nullptr);

		VKDevice(VkDevice device);

		VKDevice(const VKDevice &other) = delete;
		VKDevice(VKDevice &&other) = delete;
		~VKDevice();

		/**
		 * @brief
		 *
		 * @return true
		 * @return false
		 */
		bool isGroupDevice() const noexcept { return this->getPhysicalDevices().size() > 0; }

		/**
		 * @brief Get the Physical Devices object
		 *
		 * @return const std::vector<std::shared_ptr<PhysicalDevice>>&
		 */
		const std::vector<std::shared_ptr<PhysicalDevice>> &getPhysicalDevices() const noexcept {
			return this->physicalDevices;
		}

		/**
		 * @brief Get the Nr Physical Devices object
		 *
		 * @return unsigned int
		 */
		size_t getNrPhysicalDevices() const noexcept { return this->physicalDevices.size(); }

		/**
		 * @brief Get the Physical Device object
		 *
		 * @param index
		 * @return const std::shared_ptr<PhysicalDevice>&
		 */
		const std::shared_ptr<PhysicalDevice> &getPhysicalDevice(unsigned int index) const {
			return this->physicalDevices[index];
		}

		/**
		 * @brief Get the Handle object
		 *
		 * @return VkDevice
		 */
		VkDevice getHandle() const noexcept { return this->logicalDevice; }

		/**
		 * @brief
		 *
		 * @param typeFilter
		 * @param properties
		 * @return uint32_t
		 */
		template <size_t n = 0>
		std::optional<uint32_t> findMemoryType(const uint32_t typeFilter,
											   const VkMemoryPropertyFlags memPropertieFlags) const {
			return VKHelper::findMemoryType(physicalDevices[0]->getMemoryProperties(), typeFilter, memPropertieFlags);
		}

		/**
		 * @brief Create a Command Pool object
		 *
		 * @param queue
		 * @param flag
		 * @param pNext
		 * @return VkCommandPool
		 */
		VkCommandPool createCommandPool(const uint32_t queue,
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

		void submitCommands(
			VkQueue queue, const std::vector<VkCommandBuffer> &cmd, const std::vector<VkSemaphore> &waitSemaphores = {},
			const std::vector<VkSemaphore> &signalSempores = {}, VkFence fence = VK_NULL_HANDLE,
			const std::vector<VkPipelineStageFlags> &waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			const void *pNext = nullptr) {
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = pNext;

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
															unsigned int nrCmdBuffers = 1,
															const void *pNext = nullptr) {
			std::vector<VkCommandBuffer> cmdBuffers(nrCmdBuffers);

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.pNext = pNext;
			allocInfo.level = level;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = nrCmdBuffers;

			VKS_VALIDATE(vkAllocateCommandBuffers(this->getHandle(), &allocInfo, cmdBuffers.data()));

			return cmdBuffers;
		}

		std::vector<VkCommandBuffer> beginSingleTimeCommands(
			VkCommandPool commandPool, VkCommandBufferLevel level, const unsigned int nrCmdBuffers = 1,
			const VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			const VkCommandBufferInheritanceInfo *pInheritInfo = nullptr, const void *pNext = nullptr) {
			std::vector<VkCommandBuffer> cmd = this->allocateCommandBuffers(commandPool, level, nrCmdBuffers);

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
			this->submitCommands(queue, cmds);
			VKS_VALIDATE(vkQueueWaitIdle(queue));

			vkFreeCommandBuffers(this->getHandle(), commandPool, cmds.size(), cmds.data());
		}

		VkDeviceAddress getBufferAddress(VkBuffer buffer, const void *pNext = nullptr);

		bool isFormatSupported(const VkFormat format, const VkImageType imageType, const VkImageTiling tiling,
							   const VkImageUsageFlags usage, const VkImageCreateFlags flags = 0,
							   VkImageFormatProperties *capability = nullptr) const noexcept;

		struct VKQueue {
		  public:
			VkQueue queue;
			int familyIndex;
			int queueIndex;
		};

		VkQueue getQueue(const uint32_t queueFamilyIndex, const uint32_t queueIndex) const {
			VkQueue queue;
			vkGetDeviceQueue(this->getHandle(), queueFamilyIndex, queueIndex, &queue);
			return queue;
		}
		const std::vector<VKQueue> &getQueues() const noexcept { return this->queues; }

		template <typename T> T getProcAddress(const char *name) const noexcept {
			return (T)vkGetDeviceProcAddr(getHandle(), name);
		}

	  private:
		void createDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physicalDevices,
						  const std::unordered_map<const char *, bool> &requested_extensions,
						  const std::vector<VkDeviceQueueCreateInfo> &queues, const void *pNext = nullptr);

	  private:
		std::vector<VKQueue> queues;

		std::vector<std::shared_ptr<PhysicalDevice>> physicalDevices;
		VkDevice logicalDevice = VK_NULL_HANDLE;
	};
} // namespace fvkcore
