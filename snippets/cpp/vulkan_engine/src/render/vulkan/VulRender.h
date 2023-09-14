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

        struct EnvironmentData
        {
            glm::mat4 view;
            glm::mat4 proj;
        };

        //we used a fixed UniformBufferObject for now
        struct ObjectData
        {
            glm::mat4 model;
        };

        struct DrawableObject
        {
            enum eState {
                STATE_UNUSED = 0,
                STATE_DISABLED = 1,
                STATE_ENABLED = 2,
            };

            eState state = STATE_UNUSED;
            size_t index=0;

            std::vector<engine::VertexHandle> vertIndices;
        };

        struct VerticeList
        {
            void* ptr = nullptr;    //TODO: if != nullptr pointer is not valid
            VkBuffer buffer;
            VkDeviceMemory memory;
            std::vector<Vertex> vertices; //original vertices
        };

        enum class eDescriptorLayoutBinding: uint32_t {
            PER_ENVIRONMENT = 0,
            PER_OBJECT = 1,
        };

    public:
        Render(std::unique_ptr<Device> device, Settings settings);

        virtual engine::VertexHandle addVertex(const std::span<engine::Vertex> vertex) override;
        virtual engine::DrawableHandle addDrawable() override;
        virtual void setViewProjection(const glm::mat4& view, const glm::mat4& projection) override;
        virtual void setWorldTransform(const engine::DrawableHandle& handle, const glm::mat4& transform) override;
        virtual void setVertex(engine::DrawableHandle drawable, engine::VertexHandle vertexHandle) override;
        virtual void setEnabled(engine::DrawableHandle drawHdnl, bool isEnabled) override;
        virtual GLFWwindow& window() override;

        void draw() override;

        Device& device();

    protected:
           

    protected:
        std::unique_ptr<Device> m_device;

        void initRenderProperties();
        void initPipeline();

        void recordDraw(uint32_t imageIndex);

        Settings m_settings;

        //VkDescriptorSetLayout m_descriptor_set_layout;
        VkPipelineLayout m_pipelineLayout;
        VkPipeline m_pipeline;
        //VkDescriptorPool m_descriptorPool;

        std::vector<DrawableObject> m_drawableObject;
        std::vector<VerticeList> m_verticesOjects;

        glm::mat4 m_view;
        glm::mat4 m_projection;

        struct {
            BufferMemory environmentBuff;
            EnvironmentData* environmentBuffPtr=nullptr;

            BufferMemory objectBuff;
            ObjectData* objectBuffPtr = nullptr;
            
            VkDescriptorSet descriptorSet;
            VkDescriptorSetLayout descriptorSetLayout;
            VkDescriptorPool descriptorPool;
        } m_renderProps;
        
        
    };
}