#include "vul_base.h"

namespace vulk
{
	std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device)
	{
		uint32_t family_count = 0;
		std::vector<VkQueueFamilyProperties> queue_proterties;

		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);
		queue_proterties.resize(family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, queue_proterties.data());

		return queue_proterties;
	}

	bool getPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice device, uint32_t family_index, VkSurfaceKHR surface)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, family_index, surface, &presentSupport );
		return presentSupport;
	}

	std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		uint32_t formatCount = 0;
		std::vector<VkSurfaceFormatKHR> ret;

		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
		ret.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, ret.data());

		return ret;
	}

	std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		 uint32_t modeCount = 0;
		 std::vector<VkPresentModeKHR> ret;

		 vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);
		 ret.resize(modeCount);
		 vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, ret.data());

		 return ret;
	}


}
