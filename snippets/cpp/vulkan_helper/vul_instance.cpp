#include "vul_instance.h"

using string = std::string;

VulInstance::VulInstance(Settings settings) 
    : m_settings{settings}, m_layers_used{settings.layer}
{
    initGlfw();
    initGetExtension();
    initGetLayers();
    initInstance();
    initPhyDev();
}

void VulInstance::initGlfw() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(m_settings.width, m_settings.height, "Vulkan", nullptr, nullptr);
   
    //get extensions required for glfw
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        for(uint32_t iExt=0; iExt<glfwExtensionCount; iExt++) {
            m_extensions_used.insert(string(glfwExtensions[iExt]));
        }
    }
}

void VulInstance::initGetExtension() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    m_extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());
}

void VulInstance::initGetLayers() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    m_layers.resize(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, m_layers.data());
}

void VulInstance::initInstance() {
    //Glfw extensions
    for(const auto& iGlfwExt: m_glfw_extensions) {
        m_extensions_used.insert(iGlfwExt);
    }

    const auto [extensionsUsed, layersUsed] = std::invoke([&]{
        std::vector<const char*> extensions, layers;
        for(const auto& iExt: m_extensions_used) {
            extensions.push_back(iExt.c_str());
        }

        for(const auto& iLayer: m_layers_used) {
            layers.push_back(iLayer.c_str());
        }

        return std::tuple(extensions, layers);
    });

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
    createInfo.enabledLayerCount = layersUsed.size();
    createInfo.ppEnabledLayerNames = layersUsed.data();
    createInfo.enabledExtensionCount = extensionsUsed.size();
    createInfo.ppEnabledExtensionNames = extensionsUsed.data();

    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw string{"vkCreateInstance failed"};
    }

    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
        throw string{"failed to create window surface!"};
    }

    dumbExtensions();
}

void VulInstance::initPhyDev()
{
    std::vector<VkPhysicalDevice> physicalDevices;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance,&deviceCount, nullptr);

    if(deviceCount <= m_settings.deviceIndex)
        throw string{"device with given index not found"};

    physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
    m_physical_device = physicalDevices[m_settings.deviceIndex];

    vkGetPhysicalDeviceProperties(m_physical_device, &m_physical_device_properties);

    //surface formats
    {
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &formatCount, nullptr);
        m_surface_formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &formatCount, m_surface_formats.data());
    }

    //presentation modes
    {
        uint32_t modeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &modeCount, nullptr);
        m_present_mode.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &modeCount, m_present_mode.data());
    }

    dumpPhysicalDevice();
}

void VulInstance::initLogicDev()
{
    
}

void VulInstance::dumbExtensions() {
    printDebug("Extension:");
    for(const auto iExtensions: m_extensions) {
        const auto extName = iExtensions.extensionName;
        const auto used = std::invoke([&](){
            if(std::ranges::find(m_extensions_used, extName) != m_extensions_used.end()) {
                return string{"*"};
            }
            else {
                return string{" "};
            }
        });
       

         printDebug(std::format("   {} {}",used, extName));
    }

    printDebug("Layers:");
    for(const auto iLayer: m_layers) {
        const auto layerName = iLayer.layerName;
        const auto used = std::invoke([&](){
            if(std::ranges::find(m_layers_used, layerName) != m_layers_used.end()) {
                return string{"*"};
            }
            else {
                return string{" "};
            }
        });
       

         printDebug(std::format("   {} {}",used, layerName));
    }
}

void VulInstance::dumpPhysicalDevice() {
    printDebug(std::format("Use device {}", m_physical_device_properties.deviceName) );

    printDebug("    SurfaceFormats:");
    for(const auto iSurfaceFormat: m_surface_formats) {
         printDebug(std::format("       format={}; colorspace={}", 
            string_VkFormat(iSurfaceFormat.format),
            string_VkColorSpaceKHR(iSurfaceFormat.colorSpace)));
    }

    printDebug("    PresentMode:");
    for(const auto iPresentMode: m_present_mode) {
         printDebug(std::format("       mode={}", 
            string_VkPresentModeKHR(iPresentMode)));
    }

    
}

void VulInstance::printDebug(std::string str)
{
    std::cout << "[Vulkan] " << str << std::endl;
}