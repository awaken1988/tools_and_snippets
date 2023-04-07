#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

using std::cout;
using std::endl;
using std::string;

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
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };


    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(this->m_window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
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
            
    }


    void createSurface() {
        if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
            throw string{"failed to create window surface!"};
        }
    }

private:
    GLFWwindow* m_window;
    VkInstance  m_instance;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    std::vector<VkExtensionProperties> m_extensions;
    std::vector<const char*> m_extensionsUsed;
    std::vector<const char*> m_extensionsDeviceUsed;
    std::vector<VkLayerProperties> layers;
};