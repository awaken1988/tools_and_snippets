#pragma once

#include "VulBase.h"
#include "VulDevice.h"

#include "engine/EngineRender.h"

namespace vulk
{
    class Render : 
        public engine::Render
    {
    public:
        struct Settings
        {
            uint32_t max_objects = 1000;

            uint32_t max_descriptor_sets = 1000;
            uint32_t max_vertices_lists = 1000;
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

        class DrawableObject
        {
            friend class Render;
        public:
           
        private:
            void* mapped_ptr = nullptr; //TODO: currently we use this value to differentiate between used/not used
            VkBuffer uboBuffer;
            VkDeviceMemory uboMemory;
            VkDescriptorSet uboDescriptor;
            std::vector<engine::VertexHandle> vertIndices;
           
            size_t index;
        };

        struct VerticeList
        {
            void* ptr = nullptr;    //TODO: if != nullptr pointer is not valid
            VkBuffer buffer;
            VkDeviceMemory memory;
            std::vector<Vertex> vertices; //original vertices
        };

        struct DrawableObjectHandle
        {
            friend class Render;
        public:
            void setModelTransormation(const glm::mat4& model);
        private:
            size_t index;
            Render* renderer = nullptr; //TODO: use reference
        };

        enum class eDescriptorLayoutBinding: uint32_t {
            UBO = 0,
            TEXTURE = 1,
        };

    public:
        Render(std::unique_ptr<Device> device, Settings settings);

        virtual engine::VertexHandle addVertex(const std::span<engine::Vertex> vertex) override;
        virtual engine::DrawableHandle addDrawable() override;
        virtual void setViewProjection(const glm::mat4& view, const glm::mat4& projection) override;
        virtual void setWorldTransform(const engine::DrawableHandle& handle, const glm::mat4& transform) override;
        virtual void setVertex(engine::DrawableHandle drawable, engine::VertexHandle vertexHandle) override;
        virtual GLFWwindow& window() override;

        void draw() override;

        Device& device();

    protected:
           

    protected:
        std::unique_ptr<Device> m_device;

        void initDescriptorSetLayout();
        void initPipeline();

        void recordDraw(uint32_t imageIndex);

        Settings m_settings;

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