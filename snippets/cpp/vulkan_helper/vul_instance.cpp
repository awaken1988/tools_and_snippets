#include "vul_instance.h"

using string = std::string;

namespace vulk
{
    Device::Device(Settings settings)
        : m_settings{settings}, m_layers_used{settings.layer}
    {
        initGlfw();
        initGetExtension();
        initGetLayers();
        initInstance();
        initPhyDev();
        initLogicDev();
        initSwapchain();
    }

    void Device::initGlfw() {
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

    void Device::initGetExtension() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        m_extensions.resize(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_extensions.data());
    }

    void Device::initGetLayers() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        m_layers.resize(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, m_layers.data());
    }

    void Device::initInstance() {
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

    void Device::initPhyDev()
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

    void Device::initLogicDev()
    {
    	//get queue families
        m_queue_propterties = getPhysicalDeviceQueueFamilyProperties(m_physical_device);

        for (size_t iFamilyIndex=0; iFamilyIndex<m_queue_propterties.size(); iFamilyIndex++) {
        	const auto& property = m_queue_propterties[iFamilyIndex];

        	const bool is_graphics = property.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        	const bool is_presentation = getPhysicalDeviceSurfaceSupportKHR(m_physical_device, iFamilyIndex, m_surface);

        	//TODO: external requirements function

        	if(is_graphics) {
        		m_queue_families.graphics = iFamilyIndex;
        	}
        	if(is_presentation) {
        		m_queue_families.presentation = iFamilyIndex;
        	}
        }
        debug_print("Family Queue graphics:     {}", *m_queue_families.graphics);
        debug_print("Family Queue presentation: {}", *m_queue_families.presentation);

        //create logical device
        VkPhysicalDeviceFeatures deviceFeatures{};
        const float queuePrio = 1.0;
        const auto uniqueQueueFamilies = std::set<uint32_t>{ *m_queue_families.graphics, *m_queue_families.presentation  };
        const auto extension_used = base::ToConstCharArray{m_device_extension_used};

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
        deviceInfo.enabledExtensionCount = extension_used.size();
        deviceInfo.ppEnabledExtensionNames = extension_used.data();
        deviceInfo.enabledLayerCount = 0;

        if(vkCreateDevice(m_physical_device, &deviceInfo, nullptr, &m_logical_device)) {
            throw string{"cannot create logical Device"};
        }

        vkGetDeviceQueue(m_logical_device, *m_queue_families.graphics, 0, &m_queue.graphics);

        if(*m_queue_families.graphics == *m_queue_families.presentation) {
            vkGetDeviceQueue(m_logical_device, *m_queue_families.presentation, 0, &m_queue.presentation);
        } else {
            m_queue.presentation = m_queue.graphics;
        }
    }

    void Device::initSwapchain()
    {
    	//query all information
    	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &m_swapchain.surface_capabilities);
    	m_swapchain.surface_formats = vulk::getPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface);
    	m_swapchain.present_modes = vulk::getPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface);
    	dumpSwapchainInfo();

    }

    void Device::dumbExtensions() {
        debug_print("Extension:");
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
        

            debug_print("   {} {}",used, extName);
        }

        debug_print("Layers:");
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
        

            debug_print("   {} {}",used, layerName);
        }
    }

    void Device::dumpPhysicalDevice() {
        debug_print("Use device {}", m_physical_device_properties.deviceName);

        debug_print("    SurfaceFormats:");
        for(const auto iSurfaceFormat: m_surface_formats) {
        	debug_print("       format={}; colorspace={}",
                string_VkFormat(iSurfaceFormat.format),
                string_VkColorSpaceKHR(iSurfaceFormat.colorSpace));
        }

        debug_print("    PresentMode:");
        for(const auto iPresentMode: m_present_mode) {
            debug_print("       mode={}", 
                string_VkPresentModeKHR(iPresentMode));
        }
    }

    void Device::dumpSwapchainInfo()
    {
    	debug_print("Swapchain Info");

    	//presentation mode
    	debug_print("	presentation modes");
    	for(const auto iPresentMode: m_swapchain.present_modes) {
    		debug_print("		{}", string_VkPresentModeKHR(iPresentMode));
		}

    	debug_print("	surface formats");
    	for(const auto iSurfaceFormat: m_swapchain.surface_formats) {
			debug_print("		format={}; colorspace={}",
				string_VkFormat(iSurfaceFormat.format),
				string_VkColorSpaceKHR(iSurfaceFormat.colorSpace));
		}

    	const auto& surface_cap = m_swapchain.surface_capabilities;
    	debug_print("	surface capabilities");
    	debug_print("		image count        min={}; max={}", surface_cap.minImageCount, surface_cap.maxImageCount);
    	debug_print("		image extent min   width={}; height={}", surface_cap.minImageExtent.width, surface_cap.minImageExtent.height);
    	debug_print("		image extent max   width={}; height={}", surface_cap.maxImageExtent.width, surface_cap.maxImageExtent.height);
    }
}
