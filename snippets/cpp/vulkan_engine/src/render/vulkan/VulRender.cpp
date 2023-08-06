#include "VulRender.h"



namespace vulk
{
    Render::Render(std::unique_ptr<Device> device, Settings settings)
        : m_device{std::move(device)}, m_settings{settings}
    {
        m_drawableObject.resize(m_settings.max_objects);
        m_verticesOjects.resize(m_settings.max_vertices_lists);

        initDescriptorSetLayout();
        initPipeline();
    }

    engine::DrawableHandle Render::addDrawable()
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

        //Write DescriptorSet
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_descriptor_set_layout;

            if (vkAllocateDescriptorSets(m_device->logicalDevice(), &allocInfo, &drawable.uboDescriptor) != VK_SUCCESS) {
                throw std::string("failed to allocate descriptor sets!");
            }

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = drawable.uboBuffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 1> descriptorWrite{};
            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = drawable.uboDescriptor;
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].pImageInfo = nullptr; // Optional
            descriptorWrite[0].pTexelBufferView = nullptr; // Optional

            vkUpdateDescriptorSets(m_device->logicalDevice(), descriptorWrite.size(), descriptorWrite.data(), 0, nullptr);
        }

        return engine::DrawableHandle{index};
    }

    void Render::setViewProjection(const glm::mat4& view, const glm::mat4& projection)
    {
        //TODO: remember if we set the view,project and skip it

        for(const auto iDrawableObject: m_drawableObject) {
            if(iDrawableObject.mapped_ptr == nullptr)
                continue;

            UniformBufferObject* ubo = reinterpret_cast<UniformBufferObject*>(iDrawableObject.mapped_ptr);
            ubo->proj = projection;
            ubo->view = view;
        }
    }

    void Render::setWorldTransform(const engine::DrawableHandle& handle, const glm::mat4& transform)
    {
        auto& drawable = m_drawableObject[handle.index];
        
        UniformBufferObject* ubo = reinterpret_cast<UniformBufferObject*>(drawable.mapped_ptr);

        ubo->model = transform;
    }


    GLFWwindow& Render::window()
    {
        return device().getWindow();
    }

    engine::VertexHandle Render::addVertex(const std::span<engine::Vertex> vertex)
    {
         const auto index = std::invoke([&]() -> size_t {
            for(int iObj=0; iObj<m_verticesOjects.size(); iObj++) {
                if(m_verticesOjects[iObj].ptr==nullptr)
                    return iObj;
            }
            throw std::string{"no free vertex slot"};
        });

        VerticeList& vertObject = m_verticesOjects[index];

        //copy vertices because we use alternative format in vulkan
        //TODO: maybe we can directly use the engine:Vertex format?
        vertObject.vertices.reserve(vertex.size());
        for (const auto& iVert : vertex) {
            Vertex vulkVertex;
            vulkVertex.pos = iVert.color;
            vulkVertex.color = iVert.color;

            vertObject.vertices.push_back(vulkVertex);
        }

        const size_t verticesSize = sizeof(Vertex) * vertex.size();
        auto [vertBuffer, vertMemory] = m_device->createBuffer(
            verticesSize, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vertObject.buffer = vertBuffer;
        vertObject.memory = vertMemory;

        vkMapMemory(m_device->logicalDevice(), vertMemory, 0, verticesSize, 0, &vertObject.ptr);
        memcpy(vertObject.ptr, vertex.data(),verticesSize);
        vkUnmapMemory(m_device->logicalDevice(), vertMemory);

        return engine::VertexHandle{index};
    }

    void Render::recordDraw(uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(m_device->commandBuffer(), &beginInfo))
            throw std::string{"failed to begin recording command buffers"};

        auto swapchainData = m_device->swapchainData(imageIndex);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_device->renderpass();
        renderPassInfo.framebuffer = swapchainData.framebuffer;
        renderPassInfo.renderArea.offset = { 0,0 };
        renderPassInfo.renderArea.extent = m_device->swapChainExtent();

        VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        auto commandBuffer = m_device->commandBuffer();
        const auto swapchainExtent = m_device->swapChainExtent();

        vkCmdBeginRenderPass(commandBuffer, & renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        //dynamic viewport
        {
            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapchainExtent.width);
            viewport.height = static_cast<float>(swapchainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        }

        //dynamic scissor
        {
            VkRect2D scissor{};
            {
                decltype(swapchainExtent) scissor_size = swapchainExtent;
                //scissor_size.width /= 2;
                //scissor_size.height /= 2;

                scissor.offset = { 0, 0 };
                scissor.extent = scissor_size;
            }
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        }

        //vertex & color buffer
        
        VkDeviceSize offsets[] = { 0 };


        for(auto& iDrawableObj: m_drawableObject) {
            if (iDrawableObj.mapped_ptr == nullptr || !iDrawableObj.isEnabled) {
                continue;
            }
           
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &iDrawableObj.uboDescriptor, 0, nullptr);

            for(auto iVerticeIndice: iDrawableObj.vertIndices) {
                auto& vert = m_verticesOjects[iVerticeIndice.index];

                VkBuffer vertexBuffers[] = { vert.buffer};

                vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
                vkCmdDraw(commandBuffer, vert.vertices.size(), 1, 0, 0);
            }

        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::string("failed to record command buffer!");
        }
    }

    void Render::draw()
    {
        vkWaitForFences(m_device->logicalDevice(), 1, &m_device->inFlightFence(), VK_TRUE, UINT64_MAX);
        vkResetFences(m_device->logicalDevice(), 1, &m_device->inFlightFence());

        //wait until next image available
        const uint32_t imageIndex = std::invoke([&](){
            uint32_t ret;
            vkAcquireNextImageKHR(
                m_device->logicalDevice(), 
                m_device->swapChain(),
                UINT64_MAX,
                m_device->imageAvailableSemaphore(),
                VK_NULL_HANDLE, 
                &ret);
            return ret;
        });

        auto commandBuffer = m_device->commandBuffer();   
        vkResetCommandBuffer(m_device->commandBuffer(), 0);
        recordDraw(imageIndex);

        //submit queue
        {
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &m_device->imageAvailableSemaphore();
            
            submitInfo.pWaitDstStageMask = waitStages;
            
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_device->renderFinishedSemaphore();

            if (vkQueueSubmit(m_device->graphpicsQueue(), 1, &submitInfo, m_device->inFlightFence()) != VK_SUCCESS) {
                throw std::string("failed to submit draw command buffer!");
            }
        }

        //present image
        {
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &m_device->renderFinishedSemaphore();

            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &m_device->swapChain();
            presentInfo.pImageIndices = &imageIndex;
            presentInfo.pResults = nullptr; // Optional

            vkQueuePresentKHR(m_device->presentQueue(), &presentInfo);
        }
    }

    Device &Render::device()
    {
        return *m_device;
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
            pool.descriptorCount = m_settings.max_descriptor_sets;

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
        pipelineInfo.renderPass = m_device->renderpass();
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

    void Render::DrawableObjectHandle::setModelTransormation(const glm::mat4 &model)
    {
        auto& drawableObject = renderer->m_drawableObject[index];
        auto& gameObject = renderer->m_drawableObject[index];

        UniformBufferObject* ubo = reinterpret_cast<UniformBufferObject*>(gameObject.mapped_ptr);
        ubo->model = model;
    }

    void Render::setVertex(engine::DrawableHandle drawable, engine::VertexHandle vertexHandle)
    {
        auto& drawableObject = m_drawableObject[drawable.index];
        drawableObject.vertIndices.push_back(vertexHandle);
    }

    void Render::setEnabled(engine::DrawableHandle drawHdnl, bool isEnabled)
    {
        auto& drawObj = m_drawableObject[drawHdnl.index];
        drawObj.isEnabled = isEnabled;
    }

    //void Render::DrawableObjectHandle::addVertices(VertexHandle vertexHandle)
    //{
    //    auto& drawableObject = renderer->m_drawableObject[index];
    //    auto& gameObject = renderer->m_drawableObject[index];
    //    drawableObject.vertIndices.push_back(vertexHandle);
    //}
}