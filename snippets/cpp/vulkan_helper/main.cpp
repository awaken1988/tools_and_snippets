#include "hello.h"
#include "vul_instance.h"

#include <string>

int main() {
    const bool is_hello = false;

     try {
        if(is_hello) {
            Hello hello;
            hello.run();
        }
        else {
            VulInstance::Settings settings;
            settings.deviceIndex = 0;
            settings.layer.insert("VK_LAYER_KHRONOS_validation");
            VulInstance instance{settings};
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}