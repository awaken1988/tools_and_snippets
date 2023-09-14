#include "VulDevice.h"

using string = std::string;

namespace vulk
{
    Device::Device(Settings settings)
        : m_settings{settings}, m_layers_used{settings.layer}
    {
        m_device_extension_used.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

        initGlfw();
        initGetExtension();
        initGetLayers();
        initInstance();
        initPhyDev();
        initLogicDev();
        initSwapchain();
        initSyncObjects();
        initCommandBuffers();
        initRenderpass();
        createFramebuffer();
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

        //VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {};
        //shader_draw_parameters_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
        //shader_draw_parameters_features.pNext = nullptr;
        //shader_draw_parameters_features.shaderDrawParameters = VK_TRUE;

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceInfo.queueCreateInfoCount = queueInfos.size();
        deviceInfo.pQueueCreateInfos = queueInfos.data();
        deviceInfo.pEnabledFeatures = &deviceFeatures;
        deviceInfo.enabledExtensionCount = extension_used.size();
        deviceInfo.ppEnabledExtensionNames = extension_used.data();
        deviceInfo.enabledLayerCount = 0;
        deviceInfo.pNext = nullptr;

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

        //if(m_settings.swapchain_image_count <= m_swapchain.surface_capabilities.minImageCount
        //	|| m_settings.swapchain_image_count > m_swapchain.surface_capabilities.maxImageCount ) {
        //	throw std::string{"swapchain image count doesn't fit"};
        //}

        m_swapchain.used_surface_format = std::invoke([&] {
            for (const auto iFormat : m_swapchain.surface_formats) {
                const bool is_format = iFormat.format == VK_FORMAT_B8G8R8A8_SRGB;
                const bool is_colorspace = iFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                if (is_format && is_colorspace) {
                    return iFormat;
                }
            }
            throw std::string{"surface format not found"};
        });

        m_swapchain.used_present_mode = std::invoke([&] {
            for (const auto& iPresent : m_swapchain.present_modes) {
                if (iPresent == VK_PRESENT_MODE_FIFO_KHR) {
                    return iPresent;
                }
            }
            throw std::string{"present mode not found"};
        });

        m_swapchain.used_extent = std::invoke([&] {
            return m_swapchain.surface_capabilities.currentExtent;
        });

        m_swapchain.instance = std::invoke([&] {
            VkSwapchainKHR ret;

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = m_surface;
            createInfo.minImageCount = m_settings.swapchain_image_count;
            createInfo.imageFormat = m_swapchain.used_surface_format.format;
            createInfo.imageColorSpace = m_swapchain.used_surface_format.colorSpace;
            createInfo.imageExtent = m_swapchain.used_extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            const uint32_t queueFamilyIndices[] = {
               *m_queue_families.graphics,
               *m_queue_families.presentation,
            };

            if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0;
                createInfo.pQueueFamilyIndices = nullptr;
            }

            createInfo.preTransform = m_swapchain.surface_capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = m_swapchain.used_present_mode;
            //createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(m_logical_device, &createInfo, nullptr, &ret)) {
                throw string{ "cannot create swapchain" };
            }

            return ret;
        });

        m_swapchain.images = getSwapchainImagesKHR(m_logical_device, m_swapchain.instance);

        //image view
        for (size_t iImage = 0; iImage < m_swapchain.images.size(); iImage++) {
            auto image_view = createImageView(m_swapchain.images[iImage], m_swapchain.used_surface_format.format);
            m_swapchain.image_views.push_back(image_view);
        }

        //create framebuffer
        for (size_t iFb = 0; iFb < m_swapchain.images.size(); iFb++) {

        }
    }

    void Device::initSyncObjects() 
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateSemaphore(m_logical_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_logical_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(m_logical_device, &fenceInfo, nullptr, &m_inFlightFence) != VK_SUCCESS) 
            {
                throw string("failed to create semaphores!");
            }
    }

    void Device::initCommandBuffers()
    {
        //create pool
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = *m_queue_families.graphics;

        if (vkCreateCommandPool(m_logical_device, &poolInfo, nullptr, &m_command.pool) != VK_SUCCESS) {
            throw string("failed to create command pool!");
        }

        //create main commandBuffer
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_command.pool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_logical_device, &allocInfo, &m_command.mainBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Device::initRenderpass()
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchainImageFormat();
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

        if (vkCreateRenderPass(m_logical_device, &renderPassInfo, nullptr, &m_renderpass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    VkFormat Device::swapchainImageFormat()
    {
        return m_swapchain.used_surface_format.format;
    }

    VkDevice Device::logicalDevice()
    {
        return m_logical_device;
    }

    VkExtent2D Device::swapChainExtent()
    {
        return m_swapchain.used_extent;
    }

    VkSwapchainKHR& Device::swapChain()
    {
        return m_swapchain.instance;
    }

    VkSemaphore& Device::imageAvailableSemaphore() {
        return m_imageAvailableSemaphore;
    }
    
    VkSemaphore& Device::renderFinishedSemaphore() {
        return m_imageAvailableSemaphore;
    } 

    VkFence& Device::inFlightFence() {
        return m_inFlightFence;
    }

    VkCommandBuffer Device::commandBuffer()
    {
        return m_command.mainBuffer;
    }

    VkQueue Device::graphpicsQueue()
    {
        return m_queue.graphics;
    }

    VkQueue Device::presentQueue()
    {
        return m_queue.presentation;
    }

    Device::SwapchainData Device::swapchainData(uint32_t index)
    {
        SwapchainData data = {
            .image = m_swapchain.images[index],
            .imageView = m_swapchain.image_views[index],
            .framebuffer = m_swapchain.framebuffers[index],
        };

        return data;
    }

    VkExtent2D Device::swapchainExtent()
    {
        return m_swapchain.used_extent;
    }

    VkRenderPass Device::renderpass()
    {
        return m_renderpass;
    }

    GLFWwindow& Device::getWindow()
    {
        return *m_window;
    }

    VkImageView Device::createImageView(VkImage image, VkFormat format)
    {
        VkImageView ret;
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_logical_device, &createInfo, nullptr, &ret))
            throw std::string{"cannot create image views"};

        return ret;
    }

    BufferMemory Device::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags)
    {
        VkBuffer buffer; 
        VkDeviceMemory bufferMemory;

        VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(m_logical_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::string("failed to create buffer!");
		}

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_logical_device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_logical_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		   throw std::string("failed to allocate memory!");
	   }

       vkBindBufferMemory(m_logical_device, buffer, bufferMemory, 0);

       return BufferMemory{
           .buffer = buffer,
           .memory = bufferMemory,
           .size = size,
       };
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw string{"cannot find memory type"};
    }


    VkShaderModule Device::loadShader(std::vector<uint8_t> bytecode)
    {
        constexpr size_t min_code_size = 4;

        std::vector<uint32_t> alignedBytecode;
        alignedBytecode.resize(base::round_up(bytecode.size(), min_code_size));
        memset(alignedBytecode.data(), 0, alignedBytecode.size());
        memcpy(alignedBytecode.data(), bytecode.data(), bytecode.size());

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = alignedBytecode.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(alignedBytecode.data());
        
        VkShaderModule shaderModule;
        if(vkCreateShaderModule(m_logical_device, &createInfo, nullptr, &shaderModule))
            throw string{"create shader module failed"};

        return shaderModule;
    }

    VkShaderModule Device::loadShaderFile(std::string path)
    {
        const auto bytecode = base::readFile(path);
        auto shaderModule = loadShader(bytecode);

        return shaderModule;
    }

    void Device::createFramebuffer()
    {
        const auto framebufferCount = m_swapchain.image_views.size();

        m_swapchain.framebuffers.resize(framebufferCount);

        for (size_t i = 0; i < framebufferCount; i++) {
            VkImageView attachments[] = {
                m_swapchain.image_views[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderpass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = m_swapchain.used_extent.width;
            framebufferInfo.height = m_swapchain.used_extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_logical_device, &framebufferInfo, nullptr, &m_swapchain.framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
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
