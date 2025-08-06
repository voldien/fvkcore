#include "VkPhysicalDevice.h"
#include "vulkan/vulkan_core.h"

using namespace fvkcore;

PhysicalDevice::PhysicalDevice(VulkanCore &core, VkPhysicalDevice device) : vkCore(core) {
	this->initPhysicalDevice(device);
}

// PhysicalDevice::PhysicalDevice(VkInstance instance, VkPhysicalDevice device)
// 	: vkCore(std::make_shared<VulkanCore>(instance)) {
// 	initPhysicalDevice(device);
// }

void PhysicalDevice::initPhysicalDevice(VkPhysicalDevice device) {

	{
		/*  Get feature of the device.  */
		vkGetPhysicalDeviceFeatures(device, &this->features);

		/*  Get memory properties.   */
		vkGetPhysicalDeviceMemoryProperties(device, &this->memProperties);

		/*	Get device properties.	*/
		vkGetPhysicalDeviceProperties(device, &this->properties);
	}

	/*	*/
	{
		/*  Select queue family.    */
		uint32_t nrQueueFamilies;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &nrQueueFamilies, VK_NULL_HANDLE);
		this->queueFamilyProperties.resize(nrQueueFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &nrQueueFamilies, this->queueFamilyProperties.data());
	}

	/*	*/
	{
		uint32_t nrDeviceExtensions;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &nrDeviceExtensions, nullptr);
		this->extensions.resize(nrDeviceExtensions);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &nrDeviceExtensions, this->extensions.data());
	}

	this->mdevice = device;
}

bool PhysicalDevice::isFormatSupported(const VkFormat format, const VkImageType imageType, const VkImageTiling tiling,
									   const VkImageUsageFlags usage, const VkImageCreateFlags flags,
									   VkImageFormatProperties *PimageFormatProperties) const {

	VkImageFormatProperties prop;
	if (PimageFormatProperties == nullptr) {
		PimageFormatProperties = &prop;
	}

	VkPhysicalDeviceImageFormatInfo2 deviceFormatInfo;
	deviceFormatInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2;
	deviceFormatInfo.pNext = nullptr;
	deviceFormatInfo.type = imageType;
	deviceFormatInfo.tiling = tiling;
	deviceFormatInfo.usage = usage;
	deviceFormatInfo.flags = flags;

	VkImageFormatProperties2 imageFormatPropertie;
	imageFormatPropertie.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2;
	imageFormatPropertie.pNext = nullptr;

	VkResult result =
		vkGetPhysicalDeviceImageFormatProperties2(this->getHandle(), &deviceFormatInfo, &imageFormatPropertie);

	if (result == VK_SUCCESS) {
		return true;
	} else if (result == VK_ERROR_FORMAT_NOT_SUPPORTED || result == VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR) {
		return false;
	} else {
		VKS_VALIDATE(result);
		return false;
	}
}

void PhysicalDevice::getFormatProperties(VkFormat format, VkFormatProperties &props) const noexcept {
	vkGetPhysicalDeviceFormatProperties(this->getHandle(), format, &props);
}

bool PhysicalDevice::isPresentable(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const {
	VkBool32 present_supported{VK_FALSE};

	if (surface != VK_NULL_HANDLE) {
		VKS_VALIDATE(
			vkGetPhysicalDeviceSurfaceSupportKHR(this->getHandle(), queueFamilyIndex, surface, &present_supported));
	}

	return present_supported;
}

const char *PhysicalDevice::getDeviceName() const noexcept { return this->properties.deviceName; }