#include "vul_render.h"



namespace vulk
{
    Render::Render(std::unique_ptr<Device> device, Settings settings)
        : m_device{std::move(device)}, m_settings{settings}
    {
        m_drawableObject.resize(m_settings.max_objects);
        m_verticesOjects.resize(m_settings.max_vertices_lists);

        initRenderpass();
        initDescriptorSetLayout();
        initPipeline();
    }

    Render::DrawableObjectHandle Render::allocateDrawableSlot()
    {
        const auto index = std::invoke([&]() -> size_t {
            for(int iObj=0; iObj<m_drawableObject.size(); iObj++) {
                if(m_drawableObject[iObj].mapped_ptr==nullptr)
                    return iObj;
            }
            throw std::string{"no free drawable slot"};
        });

        DrawableObject& drawable = m_drawableObject[index];

        //create UBO + map
        {
            const size_t uboSize = sizeof(UniformBufferObject);
            auto [uboBuffer, uboMemory] = m_device->createBuffer(
                uboSize, 
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            drawable.uboBuffer = uboBuffer;
            drawable.uboMemory = uboMemory;

            if(vkMapMemory(m_device->logicalDevice(), drawable.uboMemory, 0, uboSize, 0, &drawable.mapped_ptr)) {
                throw std::string{"mapping ubo to memory failed"};
            }
        }
    
        return DrawableObjectHandle{.index = index};
    }

    Render::VertexHandle Render::addVertexList(std::vector<Vertex> vertices)
    {
         const auto index = std::invoke([&]() -> size_t {
            for(int iObj=0; iObj<m_drawableObject.size(); iObj++) {
                if(m_drawableObject[iObj].mapped_ptr==nullptr)
                    return iObj;
            }
            throw std::string{"no free drawable slot"};
        });

        VerticeList& vertObject = m_verticesOjects[index];

        const size_t verticesSize = sizeof(Vertex) * vertices.size();
        auto [vertBuffer, vertMemory] = m_device->createBuffer(
            verticesSize, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkMapMemory(m_device->logicalDevice(), vertMemory, 0, verticesSize, 0, &vertObject.ptr);
        memcpy(vertObject.ptr, vertices.data(),verticesSize);
        vkUnmapMemory(m_device->logicalDevice(), vertMemory);

        return VertexHandle{.index = index};
    }

    void Render::initRenderpass()
    {
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_device->swapchainImageFormat();
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

        if (vkCreateRenderPass(m_device->logicalDevice(), &renderPassInfo, nullptr, &m_renderpass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void Render::initDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> layoutBidnings;
        std::vector<VkDescriptorPoolSize> poolSizes;

        //UBO
        {
            const auto descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            VkDescriptorSetLayoutBinding layout{};
            layout.binding = static_cast<uint32_t>(eDescriptorLayoutBinding::UBO);
            layout.descriptorType = descriptorType;
            layout.descriptorCount = 1;
            layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            layout.pImmutableSamplers = nullptr; // Optional

            VkDescriptorPoolSize pool{};
            pool.type = descriptorType;
            pool.descriptorCount = 1;

            layoutBidnings.push_back(layout);
            poolSizes.push_back(pool);
        }

        //Texture
        if(false){
            const auto descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            VkDescriptorSetLayoutBinding layout{};
            layout.binding = static_cast<uint32_t>(eDescriptorLayoutBinding::TEXTURE);
            layout.descriptorCount = 1;
            layout.descriptorType = descriptorType;
            layout.pImmutableSamplers = nullptr;
            layout.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorPoolSize pool{};
            pool.type = descriptorType;
            pool.descriptorCount = 1;

            layoutBidnings.push_back(layout);
            poolSizes.push_back(pool);
        }

        // create layout
        {
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = layoutBidnings.size();
            layoutInfo.pBindings = layoutBidnings.data();

            if (vkCreateDescriptorSetLayout(m_device->logicalDevice(), &layoutInfo, nullptr, &m_descriptor_set_layout) != VK_SUCCESS) {
                throw std::string("failed to create descriptor set layout!");
            }
        }

        // create pool
        {
            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = poolSizes.size();
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = m_settings.max_descriptor_sets;

            if (vkCreateDescriptorPool(m_device->logicalDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
                throw std::string("failed to create descriptor pool!");
            }
        }
    }

    void Render::initPipeline()
    {
        VkShaderModule vertShaderModule = m_device->loadShaderFile("../shaders2/vert.spv");
        VkShaderModule fragShaderModule = m_device->loadShaderFile("../shaders2/frag.spv");;

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
        viewport.width = (float) m_device->swapChainExtent().width;
        viewport.height = (float) m_device->swapChainExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        //scissor
        VkRect2D scissor{};
        {
            VkExtent2D scissor_size = m_device->swapChainExtent();
            
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
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
        pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout; // Optional
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(m_device->logicalDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::string("failed to create pipeline layout!");
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
        pipelineInfo.renderPass = m_renderpass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(m_device->logicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS) {
            throw std::string("failed to create graphics pipeline!");
        }

        //end
        vkDestroyShaderModule(m_device->logicalDevice(), fragShaderModule, nullptr);
        vkDestroyShaderModule(m_device->logicalDevice(), vertShaderModule, nullptr);
    }
}