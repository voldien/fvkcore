#include "VulkanCore.h"
#include "VKHelper.h"
#include "VkPhysicalDevice.h"
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#define FMT_HEADER_ONLY
#include "VKUtil.h"
#include <fmt/core.h>
#include <getopt.h>
#include <stdexcept>

VulkanCore::VulkanCore() : inst(nullptr) {

	/*  Check for supported extensions.*/
	this->instanceExtensions = getSupportedExtensions();

	/*  Check for supported validation layers.  */
	this->instanceLayers = getSupportedLayers();
}

VulkanCore::VulkanCore(const std::unordered_map<const char *, bool> &requested_instance_extensions,
					   const std::unordered_map<const char *, bool> &requested_instance_layers, void *pNext)
	: VulkanCore() {
	Initialize(requested_instance_extensions, requested_instance_layers, pNext);
}

VulkanCore::VulkanCore(VkInstance instance) : VulkanCore() { this->inst = instance; }

void VulkanCore::Initialize(const std::unordered_map<const char *, bool> &requested_instance_extensions,
							const std::unordered_map<const char *, bool> &requested_instance_layers, void *pNext) {

	std::vector<const char *> usedInstanceExtensionNames = {
		/*	*/
		//		VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_KHR_DISPLAY_EXTENSION_NAME,
	};
	std::vector<const char *> useValidationLayers;

	/*  Check if exists.    */
	usedInstanceExtensionNames.reserve(usedInstanceExtensionNames.size() + requested_instance_extensions.size());
	for (const std::pair<const char *, bool> &n : requested_instance_extensions) {
		if (n.second) {
			if (isInstanceExtensionSupported(n.first)) {
				usedInstanceExtensionNames.push_back(n.first);
			} else
				throw cxxexcept::RuntimeException("Vulkan Instance does not support Extension: {}", n.first);
		}
	}

	/*	*/
	useValidationLayers.reserve(useValidationLayers.size() + requested_instance_layers.size());
	for (const std::pair<const char *, bool> &n : requested_instance_layers) {
		if (n.second) {
			if (isInstanceLayerSupported(n.first)) {
				useValidationLayers.push_back(n.first);
			} else
				throw cxxexcept::RuntimeException("Vulkan Instance does not support Layer: {}", n.first);
		}
	}

	/*  Get Latest Vulkan version. */
	uint32_t version;
	VKS_VALIDATE(vkEnumerateInstanceVersion(&version));

	/*	Primary Vulkan instance Object. */
	VkApplicationInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ai.pNext = VK_NULL_HANDLE;
	ai.pApplicationName = "Vulkan Sample";
	ai.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ai.pEngineName = "Vulkan Sample Engine";
	ai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ai.apiVersion = version;

	// VkDebugReportCallbackCreateInfoEXT callbackCreateInfoExt{};
	// callbackCreateInfoExt.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT; // sType
	// callbackCreateInfoExt.pNext = NULL;													   // pNext
	// callbackCreateInfoExt.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |						   // flags
	// 							  VK_DEBUG_REPORT_WARNING_BIT_EXT;
	// callbackCreateInfoExt.pfnCallback = &myDebugReportCallbackEXT; // myOutputDebugString, // pfnCallback
	// callbackCreateInfoExt.pUserData = VK_NULL_HANDLE;			   // pUserData
	// VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoExt{};
	// debugUtilsMessengerCreateInfoExt.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	// debugUtilsMessengerCreateInfoExt.pNext = &callbackCreateInfoExt;
	// VkValidationCheckEXT validationCheckExt[] = {VK_VALIDATION_CHECK_ALL_EXT};
	// VkValidationFlagsEXT validationFlagsExt{};

	// validationFlagsExt.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
	// validationFlagsExt.pNext = pNext; //&debugUtilsMessengerCreateInfoExt,
	// validationFlagsExt.disabledValidationCheckCount = 1;
	// validationFlagsExt.pDisabledValidationChecks = validationCheckExt;

	/*	Prepare the instance object. */
	VkInstanceCreateInfo ici = {};
	ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ici.pNext = pNext;
	ici.flags = 0;
	ici.pApplicationInfo = &ai;
	/*	*/
	ici.enabledLayerCount = useValidationLayers.size();
	ici.ppEnabledLayerNames = useValidationLayers.data();
	/*	*/
	ici.enabledExtensionCount = usedInstanceExtensionNames.size();
	ici.ppEnabledExtensionNames = usedInstanceExtensionNames.data();

	/*	Create Vulkan instance.	*/
	VKS_VALIDATE(vkCreateInstance(&ici, VK_NULL_HANDLE, &inst));

	/*	Get number of physical devices. */
	uint32_t nrPhysicalDevices;
	VKS_VALIDATE(vkEnumeratePhysicalDevices(this->inst, &nrPhysicalDevices, VK_NULL_HANDLE));

	/*  Get all physical devices.    */
	physicalDevices.resize(nrPhysicalDevices);
	VKS_VALIDATE(vkEnumeratePhysicalDevices(this->inst, &nrPhysicalDevices, &this->physicalDevices[0]));

	/*	*/
	uint32_t nrPhysicalDeviceGroupCount;
	VKS_VALIDATE(vkEnumeratePhysicalDeviceGroups(this->inst, &nrPhysicalDeviceGroupCount, VK_NULL_HANDLE));

	/*	TODO only if extension being requested.	*/
	std::vector<VkPhysicalDeviceGroupProperties> phyiscalGroupDevices(nrPhysicalDeviceGroupCount);
	VKS_VALIDATE(vkEnumeratePhysicalDeviceGroups(this->inst, &nrPhysicalDeviceGroupCount, phyiscalGroupDevices.data()));
}

std::vector<std::shared_ptr<PhysicalDevice>> VulkanCore::createPhysicalDevices() const {
	std::vector<std::shared_ptr<PhysicalDevice>> _physicalDevices(getPhysicalDevices().size());
	for (uint32_t i = 0; i < getPhysicalDevices().size(); i++) {
		_physicalDevices[i] = std::move(createPhysicalDevice(i));
	}
	return _physicalDevices;
}

std::shared_ptr<PhysicalDevice> VulkanCore::createPhysicalDevice(unsigned int index) const {
	return std::make_shared<PhysicalDevice>(getHandle(), getPhysicalDevices()[index]);
}

VulkanCore::~VulkanCore() {
	if (this->inst)
		vkDestroyInstance(this->inst, nullptr);
	this->inst = VK_NULL_HANDLE;
}

std::vector<VkExtensionProperties> VulkanCore::getSupportedExtensions() {

	std::vector<VkExtensionProperties> instanceExtensions;

	uint32_t extensionCount = 0;
	VKS_VALIDATE(vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extensionCount, VK_NULL_HANDLE));
	instanceExtensions.resize(extensionCount);
	VKS_VALIDATE(vkEnumerateInstanceExtensionProperties(VK_NULL_HANDLE, &extensionCount, instanceExtensions.data()));
	return instanceExtensions;
}
std::vector<VkLayerProperties> VulkanCore::getSupportedLayers() {
	std::vector<VkLayerProperties> instanceLayers;
	uint32_t layerCount = 0;
	VKS_VALIDATE(vkEnumerateInstanceLayerProperties(&layerCount, VK_NULL_HANDLE));
	instanceLayers.resize(layerCount);
	VKS_VALIDATE(vkEnumerateInstanceLayerProperties(&layerCount, instanceLayers.data()));

	return instanceLayers;
}