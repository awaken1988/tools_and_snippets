#include "hello.h"
#include "vul_device.h"
#include "vul_render.h"

#include <string>
#include <memory>

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
                .swapchain_image_count = 2,
            };
            vulk::Render::Settings rendererSettings{};

            
            auto device = std::make_unique<vulk::Device>(deviceSettings);
            
            vulk::Render renderer{
                std::move(device), 
                rendererSettings};
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
