#include "hello.h"
#include "vul_base.h"
#include "vul_device.h"
#include "vul_render.h"

#include <string>
#include <memory>
#include <chrono>
#include <iostream>


namespace primitive
{
    const auto l = -0.5f;
    const auto h = 0.5f;

    std::vector<vulk::Render::Vertex> trianglePrimitive()
    {
        return {
            {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 1.0f},  {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 1.0f},   {1.0f, 1.0f, 1.0f}},
        };
    }

    std::vector<vulk::Render::Vertex> rectanglePrimitive()
    {
        return {
             {{l, l, 1.0f}, {1.0f, 0.0f, 0.0f}},
             {{h, l, 1.0f}, {0.0f, 1.0f, 0.0f}},
             {{h, h, 1.0f}, {1.0f, 1.0f, 1.0f}},

             {{h, h, 1.0f}, {1.0f, 1.0f, 1.0f}},
             {{l, h, 1.0f}, {1.0f, 1.0f, 1.0f}},
             {{l, l, 1.0f}, {1.0f, 0.0f, 0.0f}},
        };
    }
}

struct Framecounter
{
    using tClock = std::chrono::steady_clock;

    tClock::time_point m_last;
    size_t m_count = 0;

    Framecounter()
        : m_last{tClock::now()}
    {}

    void next() {
        m_count++;

        //TODO: in seconds in double
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(tClock::now() - m_last);
        const bool is_overflow = elapsed.count() > 5'000'000;

        if (is_overflow) {
            const auto frames_per_second = static_cast<double>(m_count) / (elapsed.count() / 1'000'000);
            std::cout << frames_per_second << std::endl;
            m_count = 0;
            m_last = tClock::now();
        }
    }
};


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

            auto default_vertice = renderer.addVertices(primitive::rectanglePrimitive());

            for (int x = 0; x < 20; x++) {
                for (int y = 0; y < 20; y++) {
                    auto drawObject = renderer.addGameObject();
                    const auto swapChainExtents = renderer.device().swapChainExtent();
                    //glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                    //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    //glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtents.width / (float) swapChainExtents.height, 0.1f, 10.0f);

                    glm::mat4 model = glm::translate(glm::mat4{10.f}, glm::vec3(10.0f-x * 1.1f, 10.0f - y*1.1f, 0.0f));
                    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                    glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtents.width / (float)swapChainExtents.height, 0.1f, 60.0f);

                    proj[1][1] *= -1;

                    renderer.setViewProjection(view, proj);

                    drawObject.addVertices(default_vertice);
                    drawObject.setModelTransormation(model);
                }
            }
          
            Framecounter frames;

            while (!glfwWindowShouldClose(&renderer.device().getWindow())) {
                glfwPollEvents();
                renderer.draw();
                frames.next();
            }
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
