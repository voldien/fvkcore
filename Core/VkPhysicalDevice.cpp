#include "VkPhysicalDevice.h"
#include "VKHelper.h"
#include <memory>

PhysicalDevice::PhysicalDevice(VulkanCore &core, VkPhysicalDevice device) : vkCore(core) { initPhysicalDevice(device); }

// PhysicalDevice::PhysicalDevice(VkInstance instance, VkPhysicalDevice device)
// 	: vkCore(std::make_shared<VulkanCore>(instance)) {
// 	initPhysicalDevice(device);
// }

void PhysicalDevice::initPhysicalDevice(VkPhysicalDevice device) {

	/*  Get feature of the device.  */
	vkGetPhysicalDeviceFeatures(device, &this->features);

	/*  Get memory properties.   */
	vkGetPhysicalDeviceMemoryProperties(device, &this->memProperties);

	/*	Get device properties.	*/
	vkGetPhysicalDeviceProperties(device, &this->properties);

	/*  Select queue family.    */
	uint32_t nrQueueFamilies;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &nrQueueFamilies, VK_NULL_HANDLE);
	this->queueFamilyProperties.resize(nrQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &nrQueueFamilies, this->queueFamilyProperties.data());

	uint32_t nrExtensions;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nrExtensions, nullptr);
	this->extensions.resize(nrExtensions);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &nrExtensions, this->extensions.data());

	this->mdevice = device;
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