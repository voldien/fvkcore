#include "VKDevice.h"
#include "Exception.hpp"

using namespace fvkcore;

VKDevice::VKDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
				   const std::unordered_map<const char *, bool> &requested_extensions, VkQueueFlags requiredQueues,
				   const void *pNext) {

	/*  Select queue with graphic.  */
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t computeQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	uint32_t transferQueueNodeIndex = UINT32_MAX;

	/*	*/
	uint32_t nrQueues = 1;
	for (size_t j = 0; j < physical_devices.size(); j++) {
		const std::shared_ptr<PhysicalDevice> &phDevice = physical_devices[j];

		for (size_t i = 0; i < phDevice->getQueueFamilyProperties().size(); i++) {
			/*  */
			const VkQueueFamilyProperties &familyProp = phDevice->getQueueFamilyProperties()[i];

			/*  */
			if ((familyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 && (requiredQueues & VK_QUEUE_GRAPHICS_BIT) != 0) {
				if (graphicsQueueNodeIndex == UINT32_MAX) {
					graphicsQueueNodeIndex = i;
				}
			}
			if ((familyProp.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 && (requiredQueues & VK_QUEUE_COMPUTE_BIT) != 0) {
				if (computeQueueNodeIndex == UINT32_MAX) {
					computeQueueNodeIndex = i;
				}
			}
		}
	}

	// TODO resolve in case compute is not supported on the graphic queue.
	if ((requiredQueues & VK_QUEUE_COMPUTE_BIT) && computeQueueNodeIndex == UINT32_MAX) {
	}
	// TODO check so tha all the required required queues were found.

	this->graphics_queue_node_index = graphicsQueueNodeIndex;
	this->compute_queue_node_index = computeQueueNodeIndex;
	this->present_queue_node_index = this->graphics_queue_node_index;
	this->transfer_queue_node_index = this->graphics_queue_node_index;

	std::vector<VkDeviceQueueCreateInfo> queueCreations(nrQueues);
	std::vector<float> queuePriorities(1.0f, nrQueues);
	for (size_t i = 0; i < queueCreations.size(); i++) {
		VkDeviceQueueCreateInfo &queueCreateInfo = queueCreations[i];
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = this->graphics_queue_node_index;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriorities[i];
	}

	VKDevice(physical_devices, requested_extensions, queueCreations);
}

VKDevice::VKDevice(const std::shared_ptr<PhysicalDevice> &physicalDevice,
				   const std::unordered_map<const char *, bool> &requested_extensions, VkQueueFlags requiredQueues,
				   const void *pNext) {
	/*	*/
	const std::vector<std::shared_ptr<PhysicalDevice>> physical = {physicalDevice};
	/*	*/
	VKDevice(physical, requested_extensions, requiredQueues);
}

VKDevice::VKDevice(const std::vector<std::shared_ptr<PhysicalDevice>> &physical_devices,
				   const std::unordered_map<const char *, bool> &requested_extensions,
				   const std::vector<VkDeviceQueueCreateInfo> &queueCreations, const void *pNext) {
	/*  Required extensions.    */
	std::vector<const char *> deviceExtensions;
	deviceExtensions.reserve(requested_extensions.size());

	/*	*/
	for (size_t j = 0; j < physical_devices.size(); j++) {
		const std::shared_ptr<PhysicalDevice> &device = physical_devices[j];
		/*	Iterate through each extension and add if supported.	*/
		for (const std::pair<const char *, bool> n : requested_extensions) {
			if (n.second) {
				if (device->isExtensionSupported(n.first))
					deviceExtensions.push_back(n.first);
				else
					throw cxxexcept::RuntimeException("Device '{}' does not support: {}\n", device->getDeviceName(),
													  n.first);
			}
		}
	}

	/*	*/
	VkDeviceGroupDeviceCreateInfo deviceGroupDeviceCreateInfo{};
	deviceGroupDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;

	/*	*/
	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = pNext;
	deviceInfo.queueCreateInfoCount = queueCreations.size();
	deviceInfo.pQueueCreateInfos = queueCreations.data();

	/*	Enable group if supported.	*/
	{
		std::vector<VkPhysicalDevice> groupDevices(physical_devices.size());

		if (physical_devices.size() > 1) {

			for (size_t i = 0; i < physical_devices.size(); i++) {
				groupDevices[i] = physical_devices[i]->getHandle();
			}

			deviceGroupDeviceCreateInfo.physicalDeviceCount = groupDevices.size();
			deviceGroupDeviceCreateInfo.pPhysicalDevices = groupDevices.data();

			/*	Add group to device creation.	*/
			deviceInfo.pNext = &deviceGroupDeviceCreateInfo;
		}
	}

	deviceInfo.enabledExtensionCount = deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	/*  Create device.  */
	VKS_VALIDATE(vkCreateDevice(physical_devices[0]->getHandle(), &deviceInfo, VK_NULL_HANDLE, &this->logicalDevice));

	///*  Get all queues.    */
	// if (requiredQueues & VK_QUEUE_GRAPHICS_BIT) {
	//	vkGetDeviceQueue(this->getHandle(), this->graphics_queue_node_index, 0, &this->graphicsQueue);
	//}
	// if (requiredQueues & VK_QUEUE_GRAPHICS_BIT) {
	//	vkGetDeviceQueue(this->getHandle(), this->present_queue_node_index, 0, &this->presentQueue);
	//}
	// if (requiredQueues & VK_QUEUE_COMPUTE_BIT)
	//	vkGetDeviceQueue(this->getHandle(), this->compute_queue_node_index, 0, &this->computeQueue);
	// if (requiredQueues & VK_QUEUE_TRANSFER_BIT)
	//	vkGetDeviceQueue(this->getHandle(), this->transfer_queue_node_index, 0, &this->transferQueue);

	this->physicalDevices = physical_devices;
}

VKDevice::~VKDevice() {
	if (this->getHandle() != VK_NULL_HANDLE) {
		vkDestroyDevice(this->getHandle(), VK_NULL_HANDLE);
	}
}