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
#include "VKUtil.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace fvkcore {

	// TODO add namespace
	class PhysicalDevice;
	/**
	 * @brief
	 *
	 */
	class FVK_DECL_EXTERN VulkanCore {
	  protected:
		VulkanCore();

	  public:
		VulkanCore(const std::unordered_map<const char *, bool> &requested_instance_extensions,
				   const std::unordered_map<const char *, bool> &requested_instance_layers =
					   {{"VK_LAYER_KHRONOS_validation", true}},
				   void *pNext = nullptr);

		// VulkanCore(VkInstanceCreateInfo& c);

		template <typename T>
		VulkanCore(const std::vector<std::string> &requested_instance_extensions,
				   const std::vector<std::string> &requested_layers, const std::string &Name, uint32_t version,
				   const std::string &engine, unsigned int engineVersion, uint32_t vulkanVersion, VkStructureType type,
				   T &creationNext) noexcept
			: VulkanCore() {}
		VulkanCore(VkInstance instance);
		VulkanCore(const VulkanCore &other) = delete;
		VulkanCore(VulkanCore &&other) = delete;
		virtual ~VulkanCore();

		VulkanCore &operator=(const VulkanCore &) = delete;
		VulkanCore &operator=(VulkanCore &&) = delete;

		virtual void Initialize(const std::unordered_map<const char *, bool> &requested_instance_extensions,
								const std::unordered_map<const char *, bool> &requested_instance_layers,
								void *pNext = nullptr);

		const std::vector<VkExtensionProperties> &getInstanceExtensions() const noexcept {
			return this->instanceExtensions;
		}

		const std::vector<VkLayerProperties> &getInstanceLayers() const noexcept { return this->instanceLayers; }

		bool isInstanceExtensionSupported(const std::string &extension) const {
			return isInstanceExtensionSupported(this->getInstanceExtensions(), extension);
		}

		bool isInstanceLayerSupported(const std::string &layer) const {
			return this->isInstanceLayerSupported(this->getInstanceLayers(), layer);
		}

		/**
		 * @brief Get the Physical Devices object
		 *
		 * @return const std::vector<VkPhysicalDevice>&
		 */
		const std::vector<VkPhysicalDevice> &getPhysicalDevices() const noexcept { return this->physicalDevices; }

		uint32_t getNrPhysicalDevices() const noexcept { return getPhysicalDevices().size(); }

		/**
		 * @brief Get the Handle object
		 *
		 * @return VkInstance
		 */
		virtual VkInstance getHandle() const noexcept { return this->inst; }

		/**
		 * @brief Get the Device Group Properties object
		 *
		 * @return std::vector<VkPhysicalDeviceGroupProperties>
		 */
		std::vector<VkPhysicalDeviceGroupProperties> getDeviceGroupProperties() const {

			uint32_t nrGroups;
			VKS_VALIDATE(vkEnumeratePhysicalDeviceGroups(this->getHandle(), &nrGroups, nullptr));
			std::vector<VkPhysicalDeviceGroupProperties> prop(nrGroups);
			if (nrGroups > 0)
				VKS_VALIDATE(vkEnumeratePhysicalDeviceGroups(this->getHandle(), &nrGroups, prop.data()));
			return prop;
		}

		/**
		 * @brief Create a Physical Devices object
		 *
		 * @return std::vector<PhysicalDevice *>
		 */
		std::vector<std::shared_ptr<PhysicalDevice>> createPhysicalDevices() const;

		/**
		 * @brief Create a Physical Device object
		 *
		 * @param index
		 * @return PhysicalDevice*
		 */
		std::shared_ptr<PhysicalDevice> createPhysicalDevice(unsigned int index) const;

	  public:
		/**
		 * @brief
		 *
		 * @param extensions
		 * @param extension
		 * @return true
		 * @return false
		 */
		static bool isInstanceExtensionSupported(const std::vector<VkExtensionProperties> &extensions,
												 const std::string &extension) {
			return std::find_if(extensions.begin(), extensions.end(),
								[extension](const VkExtensionProperties &device_extension) {
									return std::strcmp(device_extension.extensionName, extension.c_str()) == 0;
								}) != extensions.cend();
		}

		/**
		 * @brief
		 *
		 * @param layers
		 * @param layer
		 * @return true
		 * @return false
		 */
		static bool isInstanceLayerSupported(const std::vector<VkLayerProperties> &layers, const std::string &layer) {
			return std::find_if(layers.begin(), layers.end(), [layer](const VkLayerProperties &device_layers) {
					   return std::strcmp(device_layers.layerName, layer.c_str()) == 0;
				   }) != layers.cend();
		}

		/**
		 * @brief Get the Supported Extensions object
		 *
		 * @return std::vector<VkExtensionProperties>
		 */
		static std::vector<VkExtensionProperties> getSupportedExtensions();

		/**
		 * @brief Get the Supported Layers object
		 *
		 * @return std::vector<VkLayerProperties>
		 */
		static std::vector<VkLayerProperties> getSupportedLayers();

		static uint32_t getVersion() {
			uint32_t version;
			VKS_VALIDATE(vkEnumerateInstanceVersion(&version));
			return version;
		}

	  protected:
		/*	*/
		std::vector<VkExtensionProperties> instanceExtensions;
		std::vector<VkLayerProperties> instanceLayers;
		VkInstance inst;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkDebugReportCallbackEXT debugReport;

		int nrGroupDevices;

		bool enableDebugTracer;

		uint32_t queue_count;
		std::vector<VkPhysicalDevice> physicalDevices;
	};

} // namespace fvkcore
