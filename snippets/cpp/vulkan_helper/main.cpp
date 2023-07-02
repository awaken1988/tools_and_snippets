#include "hello.h"
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

            const auto vertHandle = renderer.addVertexList(semplate_vertices());
            
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
