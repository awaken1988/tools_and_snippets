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

using std::cout;
using std::endl;
using std::string;

class Hello {
public:
    static constexpr uint32_t WIDTH = 800;
    static constexpr uint32_t HEIGHT = 600;

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
            cout << "   " << props.deviceName << "; " << props.apiVersion << endl;
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        return true;
    }

private:
    GLFWwindow* window;
    VkInstance  instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    std::vector<VkExtensionProperties> extensions;
    std::vector<VkLayerProperties> layers;
};