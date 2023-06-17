#pragma once

#include "vul_base.h"
#include "vul_device.h"

namespace vulk
{
    class Render
    {
    public:
        struct Settings
        {
            uint32_t max_objects = 1;
        };

        //we used a fixed UniformBufferObject for now
        struct UniformBufferObject
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        enum class eDescriptorLayoutBinding: uint32_t {
            UBO = 0,
            TEXTURE = 1,
        };

    public:
        Render(std::unique_ptr<Device> device, Settings settings);

    protected:
        std::unique_ptr<Device> m_device;

        void initRenderpass();
        void initDescriptorSetLayout();
        void initPipeline();

        Settings m_settings;

        VkRenderPass m_renderpass;
        VkDescriptorSetLayout m_descriptor_set_layout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_pipeline;
        VkDescriptorPool m_descriptorPool;

        struct DrawObject {

        } 

    };
}