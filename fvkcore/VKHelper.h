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
#include "vulkan/vulkan_core.h"
#include <VKUtil.h>
#include <array>
#include <cassert>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace fvkcore {

	/**
	 * @brief Helper functions.
	 * Set of functions for common
	 * vulkan related.
	 *
	 */
	class FVK_DECL_EXTERN VKHelper {
	  public:
		/**
		 * @brief
		 *
		 */
		static std::optional<uint32_t> findMemoryType(VkPhysicalDevice physicalDevice, const uint32_t typeFilter,
													  const VkMemoryPropertyFlags properties) noexcept;

		/**
		 * @brief
		 *
		 */
		static std::optional<uint32_t> findMemoryType(const VkPhysicalDeviceMemoryProperties &memProperties,
													  uint32_t typeFilter, VkMemoryPropertyFlags properties) noexcept;

		/**
		 * @brief
		 */
		static void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
										  VkImageLayout newLayout, const void *pNext = nullptr) noexcept;

		static void memoryBarrier(VkCommandBuffer cmd, const VkAccessFlags a, const VkAccessFlags b,
								  const VkPipelineStageFlags src, const VkPipelineStageFlags dest,
								  const char *pNext = nullptr) noexcept {
			VkMemoryBarrier memoryBarrier = {};
			memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
			memoryBarrier.pNext = pNext;
			memoryBarrier.srcAccessMask = a;
			memoryBarrier.dstAccessMask = b;

			vkCmdPipelineBarrier(cmd, src, dest, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
		}

		static void bufferBarrier(VkCommandBuffer cmd, VkAccessFlags buffer_src_access, VkAccessFlags buffer_dst_access,
								  VkBuffer buffer, size_t size, size_t offset, VkPipelineStageFlags src,
								  VkPipelineStageFlags dest, const char *pNext = nullptr) noexcept {
			VkBufferMemoryBarrier bufferBarrier = {};
			bufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			bufferBarrier.pNext = pNext;
			bufferBarrier.srcAccessMask = buffer_src_access;
			bufferBarrier.dstAccessMask = buffer_dst_access;
			bufferBarrier.srcQueueFamilyIndex = 0;
			bufferBarrier.dstQueueFamilyIndex = 0;
			bufferBarrier.buffer = buffer;
			bufferBarrier.size = size;
			bufferBarrier.offset = offset;

			vkCmdPipelineBarrier(cmd, src, dest, 0, 0, nullptr, 1, &bufferBarrier, 0, nullptr);
		}

		static void imageBarrier(VkCommandBuffer cmd, VkAccessFlags image_src_access, VkAccessFlags image_dst_access,
								 VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
								 const VkImageSubresourceRange &range, VkPipelineStageFlags src,
								 VkPipelineStageFlags dest, const char *pNext = nullptr) noexcept {

			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.pNext = pNext;
			imageMemoryBarrier.oldLayout = old_layout;
			imageMemoryBarrier.newLayout = new_layout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.subresourceRange = range;
			imageMemoryBarrier.srcAccessMask = image_src_access;
			imageMemoryBarrier.dstAccessMask = image_dst_access;

			vkCmdPipelineBarrier(cmd, src, dest, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
		}

		static void createMemory(VkDevice device, VkDeviceSize size, VkMemoryPropertyFlags properties,
								 const VkMemoryRequirements &memRequirements,
								 const VkPhysicalDeviceMemoryProperties &memoryProperies, VkDeviceMemory &deviceMemory,
								 const VkAllocationCallbacks *pAllocator = nullptr, const char *pNext = nullptr) {
			/**/
			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.pNext = pNext;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = size;
			const auto typeIndex = findMemoryType(memoryProperies, memRequirements.memoryTypeBits, properties);
			if (typeIndex) {
				allocInfo.memoryTypeIndex = typeIndex.value();
			} else {
				throw cxxexcept::RuntimeException("");
			}

			/**/
			VKS_VALIDATE(vkAllocateMemory(device, &allocInfo, pAllocator, &deviceMemory));
		}

		static VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &candidates,
											VkImageTiling tiling, VkFormatFeatureFlags features) {
			for (VkFormat format : candidates) {
				VkFormatProperties props;
				vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

				/*	*/
				if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
					return format;
				} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
					return format;
				}
			}

			throw cxxexcept::RuntimeException("failed to find supported format!");
		}

		static void getImageFormatProperties() {
			// vkGetPhysicalDeviceImageFormatProperties()
		}

		/**
		 * @brief Create a Buffer object
		 */
		static void createBuffer(VkDevice device, VkDeviceSize size,
								 const VkPhysicalDeviceMemoryProperties &memoryProperies,
								 const VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
								 VkDeviceMemory &bufferMemory, const VkAllocationCallbacks *pAllocator = nullptr,
								 const void *pNext = nullptr);

		/**
		 * @brief
		 */
		static void createImage(VkDevice device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format,
								VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
								const VkPhysicalDeviceMemoryProperties &memProperties, VkImage &image,
								VkDeviceMemory &imageMemory, const VkAllocationCallbacks *pAllocator = nullptr,
								const void *pNext = nullptr);

		/**
		 * @brief Create a Image View object
		 *
		 * @param device
		 * @param image
		 * @param format
		 * @return VkImageView
		 */
		static VkImageView createImageView(VkDevice device, VkImage image, VkImageViewType imageType, VkFormat format,
										   VkImageAspectFlags aspectFlags, uint32_t mipLevels,
										   const VkAllocationCallbacks *pAllocator = nullptr,
										   const void *pNext = nullptr);

		//	template<typename T>
		static void createSampler(VkDevice device, VkSampler &sampler, const VkSamplerCreateFlags flags,
								  float maxSamplerAnisotropy = 1.0f, VkAllocationCallbacks *pAllocator = nullptr,
								  void *pNext = nullptr);

		/**
		 * @brief Create a Shader Module object
		 *
		 * @param device
		 * @param data
		 * @return VkShaderModule
		 */
		template <typename T>
		static VkShaderModule createShaderModule(VkDevice device, const std::vector<T> &data,
												 const VkAllocationCallbacks *pAllocator = nullptr,
												 const char *pNext = nullptr) {
			return createShaderModule(device, *data.data(), data.size(), pAllocator, pNext);
		}

		template <typename T>
		static VkShaderModule createShaderModule(VkDevice device, const T &data, size_t size,
												 const VkAllocationCallbacks *pAllocator = nullptr,
												 const char *pNext = nullptr) {
			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.pNext = pNext;
			createInfo.flags = 0;
			createInfo.codeSize = size * sizeof(T);
			createInfo.pCode = reinterpret_cast<const uint32_t *>(&data);

			/*	Spirv is aligned with words of 4 bytes.	*/
			assert(createInfo.codeSize % 4 == 0);

			VkShaderModule shaderModule;
			VKS_VALIDATE(vkCreateShaderModule(device, &createInfo, pAllocator, &shaderModule));

			return shaderModule;
		}

		/**
		 * @brief Create a Pipeline Layout object
		 */
		static void createPipelineLayout(VkDevice device, VkPipelineLayout &pipelineLayout,
										 const VkPipelineLayoutCreateFlags flags = 0,
										 const std::vector<VkDescriptorSetLayout> &descLayouts = {},
										 const std::vector<VkPushConstantRange> &pushConstants = {},
										 const VkAllocationCallbacks *pAllocator = nullptr, void *pNext = nullptr);

		/**
		 * @brief Create a Descriptor Set Layout object
		 */
		template <size_t n>
		static void
		createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout &descriptorSetLayout,
								  const VkDescriptorSetLayoutCreateFlags flags,
								  const std::array<VkDescriptorSetLayoutBinding, n> &descitprSetLayoutBindings,
								  const VkAllocationCallbacks *pAllocator = nullptr, void *pNext = nullptr) noexcept {

			std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindingsV(descitprSetLayoutBindings.begin(),
																				   descitprSetLayoutBindings.end());

			VKHelper::createDescriptorSetLayout(device, descriptorSetLayout, descriptorSetLayoutBindingsV, flags,
												pAllocator, pNext);
		}

		/**
		 * @brief Create a Descriptor Set Layout object
		 */
		static void
		createDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout &descriptorSetLayout,
								  const std::vector<VkDescriptorSetLayoutBinding> &descitprSetLayoutBindings,
								  const VkDescriptorSetLayoutCreateFlags flags,
								  const VkAllocationCallbacks *pAllocator = nullptr, const void *pNext = nullptr);

		static VkDescriptorPool createDescPool(VkDevice device, const std::vector<VkDescriptorPoolSize> &poolSizes = {},
											   const VkDescriptorPoolCreateFlags flags = 0, const uint32_t maxSets = 1,
											   const VkAllocationCallbacks *pAllocator = nullptr,
											   const void *pNext = nullptr);

		static void mergeDescriptorBinding(const std::vector<VkDescriptorSetLayoutBinding> &bindings,
										   std::vector<VkDescriptorSetLayoutBinding> &mergedBindings);

		static VkPipelineCache createPipelineCache(VkDevice device, const size_t size, void *pdata,
												   const VkPipelineCacheCreateFlags flags = 0,
												   const VkAllocationCallbacks *pAllocator = nullptr,
												   const void *pNext = nullptr);

		static VkPipeline createGraphicPipeline();

		static VkPipeline createComputePipeline(
			VkDevice device, VkPipelineLayout layout, const VkPipelineShaderStageCreateInfo &compShaderStageInfo,
			VkPipelineCache pipelineCache = VK_NULL_HANDLE, VkPipeline basePipelineHandle = VK_NULL_HANDLE,
			uint32_t basePipelineIndex = 0, const VkAllocationCallbacks *pAllocator = nullptr, void *pNext = nullptr) {

			VkPipeline pipeline;

			VkComputePipelineCreateInfo pipelineCreateInfo = {};
			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.pNext = pNext;
			pipelineCreateInfo.flags = 0;
			pipelineCreateInfo.stage = compShaderStageInfo;
			pipelineCreateInfo.layout = layout;
			pipelineCreateInfo.basePipelineHandle = basePipelineHandle;
			pipelineCreateInfo.basePipelineIndex = basePipelineIndex;

			VKS_VALIDATE(
				vkCreateComputePipelines(device, pipelineCache, 1, &pipelineCreateInfo, pAllocator, &pipeline));

			return pipeline;
		}

		//
		// static bool isDeviceSuitable(VkPhysicalDevice device);

		/**
		 * @brief
		 *
		 * @param devices
		 * @param selectDevices
		 * @param device_type_filter
		 */
		static void selectDefaultDevices(std::vector<VkPhysicalDevice> &devices,
										 std::vector<VkPhysicalDevice> &selectDevices,
										 uint32_t device_type_filter = VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU |
																	   VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);

		// TODO improve to accomudate the configurations.
		/**
		 * @brief
		 *
		 * @param availableFormats
		 * @return VkSurfaceFormatKHR
		 */
		static VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats,
													  const std::vector<VkSurfaceFormatKHR> &requestFormats,
													  VkColorSpaceKHR request_color_space);

		/**
		 * @brief
		 *
		 * @param availablePresentModes
		 * @param vsync
		 * @return VkPresentModeKHR
		 */
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes,
													  const std::vector<VkPresentModeKHR> &requestPresentModes);

		/**
		 * @brief
		 *
		 * @param capabilities
		 * @param actualExtent
		 * @return VkExtent2D
		 */
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D actualExtent);

		struct QueueFamilyIndices {
			int32_t graphicsFamily = -1;
			int32_t presentFamily = -1;

			bool isComplete() noexcept { return graphicsFamily != -1 && presentFamily != -1; }
		};

		/**
		 * @brief
		 *
		 * @param device
		 * @param surface
		 * @return QueueFamilyIndices
		 */
		static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

		/**
		 * @brief
		 *
		 * @param device
		 * @param queue
		 * @param commandPool
		 * @param src
		 * @param dst
		 * @param size
		 */
		static void stageBufferCopy(VkDevice device, VkQueue queue, VkCommandPool commandPool, VkBuffer src,
									VkBuffer dst, const VkDeviceSize size) {

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			VKS_VALIDATE(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKS_VALIDATE(vkBeginCommandBuffer(commandBuffer, &beginInfo));

			VkBufferCopy copyRegion{};
			copyRegion.size = size;
			vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

			VKS_VALIDATE(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			VKS_VALIDATE(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
			VKS_VALIDATE(vkQueueWaitIdle(queue));

			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		}

		static void copyBufferToImageCmd(VkCommandBuffer cmd, VkBuffer src, VkImage dst, const VkExtent3D &size,
										 const VkOffset3D &offset = {0, 0, 0}, const void *pNext = nullptr) {

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = offset;
			region.imageExtent = size;

			vkCmdCopyBufferToImage(cmd, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		}

		static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool,
													   const void *pNext = nullptr) {
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandPool = commandPool;
			allocInfo.commandBufferCount = 1;

			VkCommandBuffer commandBuffer;
			VKS_VALIDATE(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VKS_VALIDATE(vkBeginCommandBuffer(commandBuffer, &beginInfo));

			return commandBuffer;
		}

		static void endSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandBuffer commandBuffer,
										  VkCommandPool commandPool, VkFence fence = VK_NULL_HANDLE,
										  const void *pNext = nullptr) {
			VKS_VALIDATE(vkEndCommandBuffer(commandBuffer));

			VkSubmitInfo submitInfo{};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.pNext = pNext;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			VKS_VALIDATE(vkQueueSubmit(queue, 1, &submitInfo, fence));
			VKS_VALIDATE(vkQueueWaitIdle(queue));

			/*	*/
			vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
		}

		static VkSurfaceKHR createSurface([[maybe_unused]] VkInstance instance) { return VK_NULL_HANDLE; }
	};
} // namespace fvkcore
