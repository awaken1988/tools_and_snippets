#pragma once

#include "vul_base.h"

namespace vulk
{
    //! base class, vulkan, device, instance, swapchain... 
    class Device
    {
    public:
        struct Settings {
            uint32_t width=1024;
            uint32_t height=768;
            uint32_t deviceIndex=0;
            std::set<std::string> layer;
            size_t swapchain_image_count = 1;
        };

    public:
        Device(Settings settings);
        
        VkFormat swapchainImageFormat() const;
        VkDevice logicalDevice() const;
        VkExtent2D swapChainExtent() const;

        //helper
        VkImageView createImageView(VkImage image, VkFormat format);
        std::tuple<VkBuffer,VkDeviceMemory> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkShaderModule loadShader(std::vector<uint8_t> bytecode);
        VkShaderModule loadShaderFile(std::string path);
        

    protected:
        void initGlfw();
        void initGetExtension();
        void initGetLayers();
        void initInstance();
        void initPhyDev();
        void initLogicDev();
        void initSwapchain();

        void dumbExtensions();
        void dumpPhysicalDevice();
        void dumpSwapchainInfo();

        GLFWwindow* m_window = nullptr;

        VkInstance m_instance = VK_NULL_HANDLE;

        std::vector<const char*> m_glfw_extensions;
        std::vector<VkExtensionProperties> m_extensions;
        std::vector<VkLayerProperties> m_layers;
        std::set<std::string> m_extensions_used;
        std::set<std::string> m_layers_used;
        std::set<std::string> m_device_extension_used;

        VkSurfaceKHR m_surface;

        VkPhysicalDevice m_physical_device;
        VkPhysicalDeviceProperties m_physical_device_properties;

        std::vector<VkSurfaceFormatKHR> m_surface_formats;
        std::vector<VkPresentModeKHR> m_present_mode;

        std::vector<VkQueueFamilyProperties> m_queue_propterties;

        struct {
        	std::optional<uint32_t> graphics;
        	std::optional<uint32_t> presentation;
        } m_queue_families;

        VkDevice m_logical_device;
        struct {
           	VkQueue graphics;
           	VkQueue presentation;
        } m_queue;

        struct {
        	VkSurfaceCapabilitiesKHR surface_capabilities;
        	std::vector<VkSurfaceFormatKHR> surface_formats;
        	std::vector<VkPresentModeKHR> present_modes;

            VkSurfaceFormatKHR used_surface_format;
            VkPresentModeKHR used_present_mode;
            VkExtent2D used_extent;
            
            VkSwapchainKHR instance;
            std::vector<VkImage> images;
            std::vector<VkImageView> image_views;

        } m_swapchain;

        Settings m_settings;
    };
}

