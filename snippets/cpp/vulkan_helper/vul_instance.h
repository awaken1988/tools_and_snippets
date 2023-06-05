#pragma once

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

//! base class, vulkan, device, instance, swapchain... 
class VulInstance
{
public:
    struct Settings {
        uint32_t width=1024;
        uint32_t height=768;
        uint32_t deviceIndex=0;
        std::set<std::string> layer;
    };

public:
    VulInstance(Settings settings);
protected:
    void initGlfw();
    void initGetExtension();
    void initGetLayers();
    void initInstance();
    void initPhyDev();
    void initLogicDev();

    void dumbExtensions();
    void dumpPhysicalDevice();

    void printDebug(std::string str);
    

    GLFWwindow* m_window = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;

    std::vector<const char*> m_glfw_extensions;
    std::vector<VkExtensionProperties> m_extensions;
    std::vector<VkLayerProperties> m_layers;
    std::set<std::string> m_extensions_used;
    std::set<std::string> m_layers_used;

    VkSurfaceKHR m_surface;

    VkPhysicalDevice m_physical_device;
    VkPhysicalDeviceProperties m_physical_device_properties;

    std::vector<VkSurfaceFormatKHR> m_surface_formats;
    std::vector<VkPresentModeKHR> m_present_mode;

    Settings m_settings;
};