#pragma once

#include "VulBase.h"

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

        struct SwapchainData {
            VkImage image;
            VkImageView imageView;
            VkFramebuffer framebuffer;
        };

    public:
        Device(Settings settings);
        
        VkFormat swapchainImageFormat();
        VkDevice logicalDevice();
        VkExtent2D swapChainExtent();
        VkSwapchainKHR& swapChain();

        VkSemaphore& imageAvailableSemaphore();
        VkSemaphore& renderFinishedSemaphore();
        VkFence& inFlightFence();

        VkCommandBuffer commandBuffer();

        VkQueue graphpicsQueue();
        VkQueue presentQueue();

        SwapchainData swapchainData(uint32_t index);
        VkExtent2D swapchainExtent();

        VkRenderPass renderpass();

        GLFWwindow& getWindow();

        //helper
        VkImageView createImageView(VkImage image, VkFormat format);
        std::tuple<VkBuffer,VkDeviceMemory> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        VkShaderModule loadShader(std::vector<uint8_t> bytecode);
        VkShaderModule loadShaderFile(std::string path);
        
        void createFramebuffer();

        

    protected:
        void initGlfw();
        void initGetExtension();
        void initGetLayers();
        void initInstance();
        void initPhyDev();
        void initLogicDev();
        void initSwapchain();
        void initSyncObjects();
        void initCommandBuffers();
        void initRenderpass();

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
            std::vector<VkFramebuffer> framebuffers;

        } m_swapchain;

        struct {
            VkCommandPool pool;
            VkCommandBuffer mainBuffer;
        } m_command;

        //we only support non parallel rendering for now
        VkSemaphore m_imageAvailableSemaphore;
        VkSemaphore m_renderFinishedSemaphore;
        VkFence m_inFlightFence;

        VkRenderPass m_renderpass;

        Settings m_settings;
    };
}

