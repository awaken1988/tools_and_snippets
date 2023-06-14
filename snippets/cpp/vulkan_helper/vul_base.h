#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <ranges>
#include <functional>
#include <format>
#include <iostream>
#include <optional>
#include <format>

namespace base
{
	//-----------------------------------------------------------
	// contains (for containers)
	//-----------------------------------------------------------
	template<std::ranges::input_range R, typename T>
	constexpr bool contains(R&& range, const T&& value) {
		for(const auto& i: range) {
			if(i == value)
				return true;
		}
		return false;
	}

	//-----------------------------------------------------------
	// convert string container to const char* container
	//-----------------------------------------------------------
	class ToConstCharArray
	{
	public:
		template<std::ranges::input_range R>
		ToConstCharArray(R r)
		{
			for(const auto& i: r) {
				m_strings.push_back(i);
			}
			for(const auto& i: m_strings) {
				m_cstrings.push_back(i.c_str());
			}
		}

		const char* const* data() const {
			return m_cstrings.data();
		}

		size_t size() const {
			return m_cstrings.size();
		}

	private:
		std::vector<std::string> m_strings;
		std::vector<const char*> m_cstrings;
	};
}

namespace vulk
{
	std::vector<VkQueueFamilyProperties> getPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice device);
	bool getPhysicalDeviceSurfaceSupportKHR(VkPhysicalDevice device, uint32_t family_index, VkSurfaceKHR surface);
	std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice device, VkSurfaceKHR surface);
	std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice device, VkSurfaceKHR surface);
	std::vector<VkImage> getSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain);

	template<typename... ARGS>
	void debug_print(std::string_view fmt, ARGS&&... args) {
		const auto str = std::vformat(fmt, std::make_format_args(args...));
		std::cout << "[Vulkan] " << str << std::endl;
	}

}
