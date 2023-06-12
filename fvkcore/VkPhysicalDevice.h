#pragma once
#include "VKHelper.h"
#include "VulkanCore.h"
#include <stdexcept>

namespace fvkcore {

	/**
	 * @brief
	 *
	 */
	class FVK_DECL_EXTERN PhysicalDevice {
	  public:
		PhysicalDevice(VulkanCore &core, VkPhysicalDevice device);
		// PhysicalDevice(VkInstance instance, VkPhysicalDevice device);
		PhysicalDevice(const PhysicalDevice &) = delete;
		PhysicalDevice(PhysicalDevice &&) = delete;

		const VkPhysicalDeviceFeatures &getFeatures() const noexcept { return features; }

		VkPhysicalDeviceProperties getProperties() noexcept { return properties; }
		const VkPhysicalDeviceProperties &getProperties() const noexcept { return properties; }

		VkPhysicalDeviceMemoryProperties getMemoryProperties() noexcept { return memProperties; }
		const VkPhysicalDeviceMemoryProperties &getMemoryProperties() const noexcept { return memProperties; }

		const VkPhysicalDeviceLimits &getDeviceLimits() const noexcept { return this->properties.limits; }

		inline const VkPhysicalDeviceDriverProperties getDeviceDriverProperties() {
			VkPhysicalDeviceDriverProperties devceProp;
			getProperties(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES, devceProp);
			return devceProp;
		}

		inline const VkPhysicalDeviceSubgroupProperties getDeviceSubGroupProperties() {
			VkPhysicalDeviceSubgroupProperties devceProp;
			getProperties(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES, devceProp);
			return devceProp;
		}

		unsigned int getNrQueueFamilyProperties() const noexcept { return this->queueFamilyProperties.size(); }
		/**
		 * @brief Get the Queue Family Properties object
		 * Get all the support family properties.
		 *
		 * @return const std::vector<VkQueueFamilyProperties>&
		 */
		const std::vector<VkQueueFamilyProperties> &getQueueFamilyProperties() const noexcept {
			return queueFamilyProperties;
		}

		/**
		 * @brief Check if physical device has support for any the queue types.
		 *
		 * @param queueFlag
		 * @return true
		 * @return false
		 */
		bool isQueueSupported(VkQueueFlags queueFlag) const noexcept {
			for (const VkQueueFamilyProperties &a : this->getQueueFamilyProperties()) {
				if (a.queueFlags & queueFlag) {
					return true;
				}
			}
			return false;
		}

		/**
		 * @brief
		 *
		 * @param surface
		 * @param queueFamilyIndex
		 * @return true
		 * @return false
		 */
		bool isPresentable(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;

		/**
		 * @brief
		 *
		 * @param format
		 * @param imageType
		 * @param tiling
		 * @param usage
		 * @return true
		 * @return false
		 */
		bool isFormatSupported(VkFormat format, VkImageType imageType, VkImageTiling tiling, VkImageUsageFlags usage,
							   VkImageFormatProperties *PimageFormatProperties = nullptr) const {
			VkImageFormatProperties prop;
			if (PimageFormatProperties == nullptr)
				PimageFormatProperties = &prop;
			VkResult result = vkGetPhysicalDeviceImageFormatProperties(this->getHandle(), format, imageType, tiling,
																	   usage, 0, PimageFormatProperties);
			if (result == VK_SUCCESS)
				return true;
			else if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
				return false;
			else {
				VKS_VALIDATE(result);
				return false;
			}
		}

		/**
		 * @brief Get the Format Properties object
		 *
		 * @param format
		 * @param props
		 */
		void getFormatProperties(VkFormat format, VkFormatProperties &props) const noexcept {
			vkGetPhysicalDeviceFormatProperties(this->getHandle(), format, &props);
		}

		/**
		 * @brief Get the Supported Format object
		 *
		 * @param supported
		 * @param candidates
		 * @param tiling
		 * @param features
		 * @return true
		 * @return false
		 */
		bool getSupportedFormat(VkFormat &supported, const std::vector<VkFormat> &candidates, VkImageTiling tiling,
								VkFormatFeatureFlags features) const {
			supported = VKHelper::findSupportedFormat(this->getHandle(), candidates, tiling, features);
			return candidates.size() != VK_FORMAT_UNDEFINED;
		}

		// VkFormat getSupportedFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats,
		// 							const std::vector<VkFormat> &requestFormats, VkImageTiling tiling,
		// 							VkFormatFeatureFlags features) {}

		bool isLocalandStagning() const noexcept {
			const VkPhysicalDeviceMemoryProperties &prop = getMemoryProperties();
			for (unsigned int i = 0; i < prop.memoryHeapCount; i++) {
				const VkMemoryType &memoryType = prop.memoryTypes[i];
				if (memoryType.propertyFlags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
				}
			}
			return false;
		}

		/**
		 * @brief Get the Handle object
		 *
		 * @return VkPhysicalDevice
		 */
		VkPhysicalDevice getHandle() const noexcept { return this->mdevice; }

		/**
		 * @brief Get the Extensions object
		 *
		 * @return const std::vector<VkExtensionProperties>&
		 */
		const std::vector<VkExtensionProperties> &getExtensions() const noexcept { return this->extensions; }

		/**
		 * @brief Check if extension is support by the physical device.
		 *
		 * @param extension
		 * @return true
		 * @return false
		 */
		bool isExtensionSupported(const std::string &extension) const noexcept {
			return std::find_if(this->getExtensions().begin(), this->getExtensions().end(),
								[extension](const VkExtensionProperties &device_extension) {
									return std::strcmp(device_extension.extensionName, extension.c_str()) == 0;
								}) != this->getExtensions().cend();
		}

		/**
		 * @brief
		 *
		 * @tparam T
		 * @param type
		 * @param requestFeature
		 */
		template <typename T> void checkFeature(VkStructureType type, T &requestFeature) noexcept {

			VkPhysicalDeviceFeatures2 feature = {};
			feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
			feature.pNext = &requestFeature;

			requestFeature.sType = type;
			vkGetPhysicalDeviceFeatures2(this->getHandle(), &feature);
		}

		VkPhysicalDeviceFeatures getFeature() noexcept {

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(this->getHandle(), &features);
			return features;
		}

		/**
		 * @brief Get the Properties object
		 *
		 * @tparam T
		 * @param type
		 * @param requestProperties
		 */
		template <typename T> void getProperties(VkStructureType type, T &requestProperties) noexcept {
			VkPhysicalDeviceProperties2 properties{};
			properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
			properties.pNext = &requestProperties;
			/*	*/
			requestProperties.sType = type;
			vkGetPhysicalDeviceProperties2(getHandle(), &properties);
		}

		const char *getDeviceName() const noexcept;

		/**
		 * @brief Get Vulkan instance core associated with the physical device.
		 *
		 * @return VulkanCore&
		 */
		VulkanCore &getInstance() const noexcept { return this->vkCore; }

	  protected:
		void initPhysicalDevice(VkPhysicalDevice device);

	  private:
		VkPhysicalDevice mdevice;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memProperties;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDeviceLimits limits;
		std::vector<VkQueueFamilyProperties> queueFamilyProperties;
		std::vector<VkExtensionProperties> extensions;

		VulkanCore &vkCore;
	};
} // namespace fvkcore
