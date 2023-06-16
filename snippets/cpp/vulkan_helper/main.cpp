#include "hello.h"
#include "vul_device.h"

#include <string>

int main() {
    const bool is_hello = false;

     try {
        if(is_hello) {
            Hello hello;
            hello.run();
        }
        else {
            vulk::Device::Settings settings = {
            	.deviceIndex = 0,
            	.layer = {"VK_LAYER_KHRONOS_validation"},
                .swapchain_image_count = 2,
            };
            vulk::Device instance{settings};
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
