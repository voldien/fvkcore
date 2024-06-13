#include "Exception.hpp"
#include "VKDevice.h"
#include <cstddef>
#include <cxxopts.hpp>
using namespace fvkcore;

VkBool32 debugCallBack(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
					   size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage,
					   void *pUserData) {
	return VK_TRUE;
}

int main(int argc, const char **argv) {

	/*	Default common options between all samples.	*/
	cxxopts::Options options("Example");
	options.add_options("Example")("h,help", "helper information.")("d,debug", "Debug",
																	cxxopts::value<bool>()->default_value("false"))(
		"g,gpu", "Select GPU", cxxopts::value<std::string>()->default_value("-1"));

	/*	Parse the command line input.	*/
	const auto result = options.parse(argc, (char **&)argv);

	/*	*/
	const bool debug = result["debug"].as<bool>();
	const std::string gpus = result["gpu"].as<std::string>();

	try {
		std::unordered_map<const char *, bool> required_device_extensions = {};
		std::unordered_map<const char *, bool> required_instance_layers = {{"VK_LAYER_KHRONOS_validation", true}};
		std::unordered_map<const char *, bool> required_instance_extensions = {{VK_KHR_SURFACE_EXTENSION_NAME, false},
																			   {"VK_KHR_xlib_surface", false}};

		VkValidationCheckEXT validationCheckExt[] = {VK_VALIDATION_CHECK_ALL_EXT};
		VkValidationFlagsEXT validationFlagsExt{};
		validationFlagsExt.sType = VK_STRUCTURE_TYPE_VALIDATION_FLAGS_EXT;
		validationFlagsExt.pNext = nullptr;
		validationFlagsExt.disabledValidationCheckCount = 1;
		validationFlagsExt.pDisabledValidationChecks = validationCheckExt;

		/*	Vulkan core.	*/
		std::shared_ptr<VulkanCore> core =
			std::make_shared<VulkanCore>(required_instance_extensions, required_instance_layers, &validationFlagsExt);

		/*	All physical devices.	*/
		std::vector<std::shared_ptr<PhysicalDevice>> physical_devices = core->createPhysicalDevices();

		for (size_t i = 0; i < physical_devices.size(); i++) {
			std::cout << physical_devices[i]->getDeviceName() << std::endl;
			for(size_t j = 0; j < physical_devices[i]->getQueueFamilyProperties().size(); j++){
				physical_devices[i]->getQueueFamilyProperties()[j].queueCount;
			}
		}
		std::flush(std::cout);

		for (size_t i = 0; i < physical_devices.size(); i++) {

			std::vector<VkDeviceQueueCreateInfo> queues;

			for (size_t j = 0; j < physical_devices[i]->getQueueFamilyProperties().size(); j++) {
				/*  */
				const VkQueueFamilyProperties &familyProp = physical_devices[i]->getQueueFamilyProperties()[j];
				std::vector<float> queuePriorities(physical_devices[i]->getQueueFamilyProperties().size(), 1.0f);

				VkDeviceQueueCreateInfo queueCreateInfo;
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.pNext = nullptr;
				queueCreateInfo.flags = 0;
				queueCreateInfo.queueFamilyIndex = j;
				queueCreateInfo.queueCount = familyProp.queueCount;
				queueCreateInfo.pQueuePriorities = queuePriorities.data();

				queues.push_back(queueCreateInfo);
			}

			/*	*/
			std::vector<std::shared_ptr<PhysicalDevice>> devices = {physical_devices[i]};
			std::shared_ptr<VKDevice> device = std::make_shared<VKDevice>(devices, required_device_extensions, queues);

			/*	*/
			VkQueue queue = device->getQueue(0, 0);
		}

	} catch (std::exception &ex) {
		cxxexcept::printStackMessage(ex);
	}
	return EXIT_SUCCESS;
}