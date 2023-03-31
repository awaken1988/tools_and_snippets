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

using std::cout;
using std::endl;
using std::string;

class Hello {
public:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily = 0;
    };


    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(this->window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

private:
    void initVulkan() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        this->window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

        createInstance();
        //setupDebugMessenger();
        pickPhysicalDevice();
        createLogicalDevice();
    }

    void createInstance() {
        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        std::vector<const char*> extensionsUsed;

        //add glfw extension
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        //glfw extension
        {
            cout << "Required Extensions" << endl;
            for(uint32_t iExt=0; iExt<glfwExtensionCount; iExt++) {
                cout << "   " << glfwExtensions[iExt] << endl;
            }

            extensionsUsed.insert(extensionsUsed.end(), glfwExtensions, glfwExtensions+glfwExtensionCount);
        }

        //extensions
        {
            uint32_t extensionCount = 0;
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
            extensions.resize(extensionCount);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
            cout << "Extensions" << endl;
            for(const auto& iExt: this->extensions) {
                cout << "   " << iExt.extensionName << endl;
            }   
        }

        //extension used
        {
            cout << "Extensions Used" << endl;
            for(const auto& iExt: extensionsUsed) {
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
        createInfo.enabledExtensionCount = extensionsUsed.size();
        createInfo.ppEnabledExtensionNames = extensionsUsed.data();


        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw string{"vkCreateInstance failed"};
        }
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        std::vector<VkPhysicalDevice> physicalDevices;
        
        vkEnumeratePhysicalDevices(instance,&deviceCount, nullptr);
        physicalDevices.resize(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        if(physicalDevices.size() == 0) {
            throw string{"no physicalDevices found"};
        }

        cout << "Devices Available" << endl;
        for(const auto& iDev: physicalDevices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(iDev, &props);

            string use;
            if(physicalDevice == VK_NULL_HANDLE && isDeviceSuitable(iDev)) {
                physicalDevice = iDev;
                use = "USE";
            }

            cout << "   " << props.deviceName << "; " << use << endl;
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        const auto indices = findQueueFamilies(device);
        return indices.graphicsFamily.has_value();
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
            if( iProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                break;
            }
        }

        return indices;
    }

    void createLogicalDevice() {
        const auto queueIndices = findQueueFamilies(physicalDevice);
        
        const float queuePrio = 1.0;

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = *queueIndices.graphicsFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePrio;

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = 1;
        deviceInfo.pQueueCreateInfos = &queueInfo;
        deviceInfo.pEnabledFeatures = &deviceFeatures;
        deviceInfo.enabledExtensionCount = 0;
        deviceInfo.enabledLayerCount = 0;

        if(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device)) {
            throw string{"cannot create logicalDevice"};
        }

        vkGetDeviceQueue(device, queueIndices.graphicsFamily.value(), 0, &graphicsQueue);
    }

private:
    GLFWwindow* window;
    VkInstance  instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;

    std::vector<VkExtensionProperties> extensions;
    std::vector<VkLayerProperties> layers;
};