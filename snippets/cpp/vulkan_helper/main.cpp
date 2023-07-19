#include "hello.h"
#include "vul_base.h"
#include "vul_device.h"
#include "vul_render.h"

#include <string>
#include <memory>

std::vector<vulk::Render::Vertex> semplate_vertices() 
{
    return {
        {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 1.0f},  {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 1.0f},   {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 1.0f},  {1.0f, 1.0f, 1.0f}}      
    };
}   

std::vector<vulk::Render::Vertex> semplate_vertices_2()
{
    return {
        {{-1.2f, -1.2f, 1.0f}, {1.0f, 0.0f, 0.0f}},
        {{1.0f, -1.2f, 1.0f},  {0.0f, 1.0f, 0.0f}},
        {{0.2f, 0.2f, 1.0f},   {0.0f, 0.0f, 1.0f}},
        {{-0.2f, 0.2f, 1.0f},  {1.0f, 1.0f, 1.0f}}
    };
}


int main() {
    const bool is_hello = false;

     try {
        if(is_hello) {
            Hello hello;
            hello.run();
        }
        else {
            vulk::Device::Settings deviceSettings = {
            	.deviceIndex = 0,
            	.layer = {"VK_LAYER_KHRONOS_validation"},
                .swapchain_image_count = 3,
            };
            vulk::Render::Settings rendererSettings{};

            
            auto device = std::make_unique<vulk::Device>(deviceSettings);
            
            vulk::Render renderer{
                std::move(device), 
                rendererSettings};

            //const auto vertHandle = renderer.addVertexList(semplate_vertices());

            {
                auto drawObject = renderer.addGameObject();
                const auto swapChainExtents = renderer.device().swapChainExtent();
                glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    	        glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtents.width / (float) swapChainExtents.height, 0.1f, 10.0f);
    	        proj[1][1] *= -1;

                renderer.setViewProjection(view, proj);

                drawObject.addVertices(semplate_vertices());
                drawObject.setModelTransormation(model);
            }
          
            while (!glfwWindowShouldClose(&renderer.device().getWindow())) {
                glfwPollEvents();
                renderer.draw();
            }
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
