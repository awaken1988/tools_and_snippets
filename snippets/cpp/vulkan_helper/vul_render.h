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

            uint32_t max_descriptor_sets = 100;
            uint32_t max_vertices_lists = 100;
        };

        //TODO: move vertex out of this class
        struct Vertex {
            glm::vec3 pos;
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
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, pos);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(Vertex, color);

                return attributeDescriptions;
            }
        };

        //we used a fixed UniformBufferObject for now
        struct UniformBufferObject
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        struct DrawableObject
        {
            void* mapped_ptr = nullptr;
            VkBuffer uboBuffer;
            VkDeviceMemory uboMemory;
            VkDescriptorSet uboDescriptor;
        };

        struct VerticeList
        {
            void* ptr = nullptr;    //TODO: if != nullptr pointer is not valid
            VkBuffer buffer;
            VkDeviceMemory memory;
            std::vector<Vertex> vertices //original vertices
        };

        struct DrawableObjectHandle
        {
            size_t index;
        };

        struct VertexHandle
        {
            size_t index;
        };

        enum class eDescriptorLayoutBinding: uint32_t {
            UBO = 0,
            TEXTURE = 1,
        };

    public:
        Render(std::unique_ptr<Device> device, Settings settings);


        VertexHandle addVertexList(std::vector<Vertex> vertices);   
        DrawableObjectHandle addGameObject();

        void setView(const glm::mat4& view);
        void setProjection(const glm::mat4& projection);
        void setPosition(DrawableObjectHandle handle, const glm::mat4& modelPosition);

        void draw();

    protected:
        

    protected:
        std::unique_ptr<Device> m_device;

        void initDescriptorSetLayout();
        void initPipeline();

        void recordDraw(uint32_t imageIndex);

        Settings m_settings;

        VkRenderPass m_renderpass;
        VkDescriptorSetLayout m_descriptor_set_layout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_pipeline;
        VkDescriptorPool m_descriptorPool;

        std::vector<DrawableObject> m_drawableObject;
        std::vector<VerticeList> m_verticesOjects;

        glm::mat4 m_view;
        glm::mat4 m_projection;

    };
}