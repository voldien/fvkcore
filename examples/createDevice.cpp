#include <VKDevice.h>
using namespace fvkcore;
int main(int argc, const char **argv) {

	std::unordered_map<const char *, bool> required_device_extensions = {};
	std::unordered_map<const char *, bool> required_instance_layers = {{"VK_LAYER_KHRONOS_validation", true}};
	std::unordered_map<const char *, bool> required_instance_extensions = {{VK_KHR_SURFACE_EXTENSION_NAME, false},
																		   {"VK_KHR_xlib_surface", false}};
	/*	Vulkan core.	*/
	std::shared_ptr<VulkanCore> core =
		std::make_shared<VulkanCore>(required_instance_extensions, required_instance_layers);

	/*	All physical devices.	*/
	std::vector<std::shared_ptr<PhysicalDevice>> physical_devices = core->createPhysicalDevices();

	// TODO print all selected devices!
	for (int i = 0; i < physical_devices.size(); i++)
		std::cout << physical_devices[i]->getDeviceName() << std::endl;
	std::shared_ptr<VKDevice> device = std::make_shared<VKDevice>(physical_devices);

	return EXIT_SUCCESS;
}