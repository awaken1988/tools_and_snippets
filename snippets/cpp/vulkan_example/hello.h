#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <string>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>

using std::cout;
using std::endl;
using std::string;

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);


        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

class Hello {
public:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentationFamily.has_value();
        }

        bool isDiff() const {
            return graphicsFamily.value() != presentationFamily.value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    void run() {
        initVertices();
        initVulkan();
        mainLoop();
        cleanup();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(this->m_window)) {
            glfwPollEvents();
            drawFrame();
        }

         vkDeviceWaitIdle(m_device);
    }

    void cleanup() {
    	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
        vkDestroySemaphore(m_device, m_imageAvailableSemaphore, nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphore, nullptr);
        vkDestroyFence(m_device, m_inFlightFence, nullptr);

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);

        for (auto framebuffer : m_swapChainFramebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }

        for(auto iImageView: m_swapChainImageViews) {
            vkDestroyImageView(m_device, iImageView, nullptr);
        }
        
        vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
        vkDestroyRenderPass(m_device, m_renderPass, nullptr);
        vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
        vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);
        vkDestroyDevice(m_device, nullptr);
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
        glfwDestroyWindow(this->m_window);
        glfwTerminate();

    }

private:
    void initVulkan() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        this->m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

        createInstance();
        //setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout(); //right order???
        createGraphicsPipeline();
        createFramebuffers();
        createCommandPool();
        createVertexBuffer();
        createUniformBuffers();
        createCommandBuffer();
        createSyncObjects();
    }

    void createInstance() {
        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        m_extensionsDeviceUsed.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        //add glfw extension
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        //glfw extension
        {
            cout << "Required Extensions" << endl;
            for(uint32_t iExt=0; iExt<glfwExtensionCount; iExt++) {
                cout << "   " << glfwExtensions[iExt] << endl;
            }

            m_extensionsUsed.insert(m_extensionsUsed.end(), glfwExtensions, glfwExtensions+glfwExtensionCount);
        }

        //extensions
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            m_extensions.resize(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());
            cout << "Extensions" << endl;
            for(const auto& iExt: this->m_extensions) {
                cout << "   " << iExt.extensionName << endl;
            }   
        }

        //extension used
        {
            cout << "Extensions Used" << endl;
            for(const auto& iExt: m_extensionsUsed) {
                cout << "   " << iExt << endl;
            }   
        }

        //layers
        {
            uint32_t layerCount = 0;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            layers.resize(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
            cout << "Layers" << endl;
            for(const auto& iLayer: this->layers) {
                cout << "   " << iLayer.layerName << endl;
            } 
        }

        //layers used
        {   
            
            bool found = false;
            for(const auto& iWanted: validationLayers) {
                for(const auto& iLayer: this->layers) {
                    if(std::strcmp(iWanted, iLayer.layerName) == 0) {
                        found = true;
                        break;
                    }
                }
                if(!found)
                    throw string{"layer not found "} + string{iWanted};
            }
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "hello";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
        createInfo.enabledExtensionCount = m_extensionsUsed.size();
        createInfo.ppEnabledExtensionNames = m_extensionsUsed.data();


        if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw string{"vkCreateInstance failed"};
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        std::vector<VkPhysicalDevice> physicalDevices;
        
        vkEnumeratePhysicalDevices(m_instance,&deviceCount, nullptr);
        physicalDevices.resize(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());

        if(physicalDevices.size() == 0) {
            throw string{"no physicalDevices found"};
        }

        cout << "Devices Available" << endl;
        for(const auto& iDev: physicalDevices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(iDev, &props);

            string use;
            if(m_physicalDevice == VK_NULL_HANDLE && isDeviceSuitable(iDev)) {
                m_physicalDevice = iDev;
                cout << "   " << props.deviceName << "; USE" << endl;
            }
            else {
                cout << "   " << props.deviceName << endl;
            }
        }

        if(m_physicalDevice==VK_NULL_HANDLE) {
            throw string{"no device found"};
        }

        cout << "Swapchain Infos" << endl;      
        {
            const auto swapchainSupport = querySwapChainSupport(m_physicalDevice);
            
            //cout << "   Capabilities: " << static_cast<uint64_t>(swapchainSupport.capabilities.) << endl;

            cout << "   Surface Formats" << endl;
            for(const auto& iSurfaceFormat: swapchainSupport.formats) {
                cout << "       " << std::hex << static_cast<uint64_t>(iSurfaceFormat.format) << endl;
            }

            cout << "   Present Modes" << endl;
            for(const auto& iPresentMode: swapchainSupport.presentModes) {
                cout << "       " << std::hex << static_cast<uint64_t>(iPresentMode) << endl;
            }
        }


    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        const auto indices = findQueueFamilies(device);
  
        const bool isFamily = indices.isComplete();
        const bool isExtension = checkDeviceExtensionSupport(device);

        bool isSwapChain = [this,&device](){
            const auto swapchainSupport = querySwapChainSupport(device);
            return !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
        }();

        return isFamily && isExtension && isSwapChain;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount = 0;
        std::vector<VkExtensionProperties> availExtensions;

        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        availExtensions.resize(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availExtensions.data());

        std::set<string> requiredExtensions{m_extensionsDeviceUsed.begin(), m_extensionsDeviceUsed.end()};
        for(const auto& iExtension: availExtensions) {
            requiredExtensions.erase(iExtension.extensionName);
        }

        return requiredExtensions.empty();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        std::vector<VkQueueFamilyProperties> queueProperties;

        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,nullptr);
        queueProperties.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device,&queueFamilyCount,queueProperties.data());
        
        int i = 0;
        for(const auto& iProperty: queueProperties) {
            const bool isGraphics = iProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            const bool isPresentation = [this,i,device]() {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport );
                return presentSupport;
            }();

            if( isGraphics) 
                indices.graphicsFamily = i;
            if( isPresentation )
                indices.presentationFamily = i;
        }

        return indices;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        //surface formats
        {
            uint32_t formatCount = 0;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
        }

        //presentation modes
        {
            uint32_t modeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &modeCount, nullptr);
            details.presentModes.resize(modeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &modeCount, details.presentModes.data());
        }

        return details;
    }

    void createLogicalDevice() {
        const auto queueIndices = findQueueFamilies(m_physicalDevice);
        
        cout << "graphicalIndice: " << queueIndices.graphicsFamily.value() << endl;
        cout << "presentationIndice: " << queueIndices.presentationFamily.value() << endl;

        const float queuePrio = 1.0;

        VkPhysicalDeviceFeatures deviceFeatures{};

        std::set<uint32_t> uniqueQueueFamilies = { queueIndices.graphicsFamily.value(), queueIndices.presentationFamily.value()  };

        std::vector<VkDeviceQueueCreateInfo> queueInfos;
        for(const auto& iQueueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueInfo{};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = iQueueFamily;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = &queuePrio;
            queueInfos.push_back(queueInfo);
        };

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = queueInfos.size();
        deviceInfo.pQueueCreateInfos = queueInfos.data();
        deviceInfo.pEnabledFeatures = &deviceFeatures;
        deviceInfo.enabledExtensionCount = m_extensionsDeviceUsed.size();
        deviceInfo.ppEnabledExtensionNames = m_extensionsDeviceUsed.data();
        deviceInfo.enabledLayerCount = 0;

        if(vkCreateDevice(m_physicalDevice, &deviceInfo, nullptr, &m_device)) {
            throw string{"cannot create logicalDevice"};
        }

        vkGetDeviceQueue(m_device, queueIndices.graphicsFamily.value(), 0, &m_graphicsQueue);

        if(queueIndices.graphicsFamily.value() != queueIndices.presentationFamily.value()) {
            vkGetDeviceQueue(m_device, queueIndices.presentationFamily.value(), 0, &m_presentQueue);
        } else {
            m_presentQueue = m_graphicsQueue;
        }
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& iFormat : availableFormats) {
            if (iFormat.format == VK_FORMAT_B8G8R8A8_SRGB && iFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
                return iFormat;
            }
        }
        throw string{ "no suitable surface format found" };
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width = 0;
            int height = 0;
            glfwGetFramebufferSize(m_window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width), 
                static_cast<uint32_t>(height),
            };


            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainDetails = querySwapChainSupport(m_physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainDetails.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainDetails.capabilities);

        m_swapChainImageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;

        if (swapChainDetails.capabilities.maxImageCount > 0 && imageCount > swapChainDetails.capabilities.maxImageCount) {
            imageCount = swapChainDetails.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentationFamily.value() };

        if (indices.isDiff()) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainDetails.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapChain)) {
            throw string{ "cannot create swapchain" };
        }

        uint32_t swapchainImageCount = 0;
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapchainImageCount, nullptr);
        m_swapChainImages.resize(swapchainImageCount);
        vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapchainImageCount, m_swapChainImages.data());
    }


    void createSurface() {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
            throw string{"failed to create window surface!"};
        }
    }

    void createImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());
        for(size_t i=0; i<m_swapChainImageViews.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_swapChainImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if(vkCreateImageView(m_device, &createInfo, nullptr, &m_swapChainImageViews[i]))
                throw string{"cannot create image views"};
        }
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        VkShaderModule shaderModule;
        if(vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule))
            throw string{"create shader module failed"};

        return shaderModule;
    }

    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("../shaders/vert.spv");
        auto fragShaderCode = readFile("../shaders/frag.spv");

        cout << "vertex shader size   = 0x" << vertShaderCode.size() << endl;
        cout << "fragment shader size = 0x" << fragShaderCode.size() << endl;

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        //dynamic state info
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
        
        //vertex input
        auto bindingDescriptions = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional
     

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        //viewport
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_swapChainExtent.width;
        viewport.height = (float) m_swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        //scissor
        VkRect2D scissor{};
        {
            VkExtent2D scissor_size = m_swapChainExtent;
            
            //scissor_size.height = 100;
            //scissor_size.width = 100;

            scissor.offset = {0, 0};
            scissor.extent = scissor_size;
        }
       

        //viewport
        //  VkPipelineViewportStateCreateInfo viewportState{};
        //  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        //  viewportState.viewportCount = 1;
        //  viewportState.scissorCount = 1;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f; // Optional
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1; // Optional
        pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw string("failed to create pipeline layout!");
        }

        //Pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr; // Optional
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = m_pipelineLayout;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS) {
            throw string("failed to create graphics pipeline!");
        }

        //end
        vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
        vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
    } 

    void createRenderPass() {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createFramebuffers() {
        m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                m_swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapChainExtent.width;
            framebufferInfo.height = m_swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void createCommandPool() {
        const auto queueFamilyIndices = findQueueFamilies(m_physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw string("failed to create command pool!");
        }
    }

    void createCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if(vkBeginCommandBuffer(m_commandBuffer, &beginInfo))
            throw string{"failed to begin recording command buffers"};

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = m_swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChainExtent.width);
        viewport.height = static_cast<float>(m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        {
            decltype(m_swapChainExtent) scissor_size = m_swapChainExtent;
            //scissor_size.width /= 2;
            //scissor_size.height /= 2;

            scissor.offset = {0, 0};
            scissor.extent = scissor_size;
        }
        vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);

        //vertex & color buffer
        VkBuffer vertexBuffers[] = {m_vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdDraw(m_commandBuffer, m_vertices.size(), 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw string("failed to record command buffer!");
        }
    }

    void drawFrame() {
        vkWaitForFences(m_device, 1, &m_inFlightFence, VK_TRUE, UINT64_MAX);
        vkResetFences(m_device, 1, &m_inFlightFence);

        uint32_t imageIndex;
        vkAcquireNextImageKHR(m_device, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        vkResetCommandBuffer(m_commandBuffer, 0);
        recordCommandBuffer(m_commandBuffer, imageIndex);

        updateUniformBuffer();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphore};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffer;

        VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphore};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFence) != VK_SUCCESS) {
            throw string("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {m_swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        vkQueuePresentKHR(m_presentQueue, &presentInfo);

        m_drawCalls++;
        m_fps.next();
    }

    void createSyncObjects() {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) 
            {
                throw string("failed to create semaphores!");
            }
    }

    void initVertices() {
        m_vertices = {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
    }
    
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::string("failed to create buffer!");
		}

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		   throw std::string("failed to allocate memory!");
	   }

       vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }

    void createVertexBuffer() {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(m_vertices[0]) * m_vertices.size();
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &m_vertexBuffer) != VK_SUCCESS) {
            throw std::string("failed to create vertex buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, m_vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &m_vertexBufferMemory) != VK_SUCCESS) {
            throw std::string("failed to allocate vertex buffer memory!");
        }

        vkBindBufferMemory(m_device, m_vertexBuffer, m_vertexBufferMemory, 0);
        
        void* data;
        vkMapMemory(m_device, m_vertexBufferMemory, 0, bufferInfo.size, 0, &data);
        memcpy(data, m_vertices.data(), (size_t) bufferInfo.size);
        vkUnmapMemory(m_device, m_vertexBufferMemory);
  
    }

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw string{"cannot find memory type"};
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
            throw string("failed to create descriptor set layout!");
        }
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers, m_uniformBuffersMemory);
        vkMapMemory(m_device, m_uniformBuffersMemory, 0, bufferSize, 0, &m_uniformBuffersMapped);
    }

    void updateUniformBuffer() {
    	auto currentTime = std::chrono::steady_clock::now();
    	float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_since_start).count() / 1.0f;

    	UniformBufferObject ubo{};
    	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    	ubo.proj = glm::perspective(glm::radians(45.0f), m_swapChainExtent.width / (float) m_swapChainExtent.height, 0.1f, 10.0f);
    	ubo.proj[1][1] *= -1;

    	memcpy(m_uniformBuffersMapped, &ubo, sizeof(ubo));
    }

private:
    GLFWwindow* m_window;
    VkInstance  m_instance;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    VkPipelineLayout m_pipelineLayout;
    VkDescriptorSetLayout m_descriptorSetLayout;
    VkRenderPass m_renderPass;
    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkCommandPool m_commandPool;
    VkCommandBuffer m_commandBuffer;

    VkBuffer m_uniformBuffers;
    VkDeviceMemory m_uniformBuffersMemory;
    void* m_uniformBuffersMapped;

    VkSemaphore m_imageAvailableSemaphore;
    VkSemaphore m_renderFinishedSemaphore;
    VkFence m_inFlightFence;

    std::vector<VkExtensionProperties> m_extensions;
    std::vector<const char*> m_extensionsUsed;
    std::vector<const char*> m_extensionsDeviceUsed;
    std::vector<VkLayerProperties> layers;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;

    std::vector<Vertex> m_vertices;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    uint64_t m_drawCalls = 0;

    struct fps_t {
    	uint64_t frame_count = 0;
    	std::chrono::steady_clock::time_point start;
    	void next() {
    		using namespace std::chrono;

    		if(frame_count==0)
    			start = steady_clock::now();
    		frame_count++;



    		if(duration_cast<milliseconds>(steady_clock::now()-start).count() > 1000) {
    			std::cout << std::dec << "fps = " << frame_count << std::endl;
    			frame_count = 0;
    		}
    	}
    };

    fps_t m_fps;

    std::chrono::steady_clock::time_point m_since_start;
};
