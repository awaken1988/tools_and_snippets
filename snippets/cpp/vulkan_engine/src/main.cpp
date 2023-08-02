#include "render/vulkan/VulBase.h"
#include "render/vulkan/VulDevice.h"
#include "render/vulkan/VulRender.h"

#include "engine/EngineRender.h"

#include "example/blocks/blocks.h"

struct TestStruct {
    int x = 0;
};

void giveSpan(const std::span<TestStruct> arg) {

}

void test()
{
    std::vector<TestStruct> arr;
    giveSpan(arr);
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
    const bool is_hello = true;

     try {
        if(is_hello) {
        
        }
        else {
            using namespace engine;

            auto render = Render::createInstance(RenderBackend::VULKAN);

            auto testPrimitive = primitive::rectanglePrimitive();
            auto testHandle = render->addVertex(testPrimitive);


            //const auto vertHandle = renderer.addVertexList(semplate_vertices());

            //auto rectangle = render->addVertex(primitive::rectanglePrimitive());
            //auto triangle = render->addVertex((primitive::trianglePrimitive());

            //for (int x = 0; x < 10; x++) {
            //    for (int y = 0; y < 10; y++) {
            //        auto drawObject = renderer.addGameObject();
            //        const auto swapChainExtents = renderer.device().swapChainExtent();
            //        //glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            //        //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            //        //glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtents.width / (float) swapChainExtents.height, 0.1f, 10.0f);

            //        glm::mat4 model = glm::translate(glm::mat4{10.f}, glm::vec3(10.0f-x * 2.1f, 10.0f - y*2.1f, 0.0f));
            //        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            //        glm::mat4 proj = glm::perspective(glm::radians(32.0f), swapChainExtents.width / (float)swapChainExtents.height, 0.1f, 60.0f);

            //        proj[1][1] *= -1;

            //        renderer.setViewProjection(view, proj);

            //        if ((x % 2) == 0 || (y % 2) == 0) {
            //            drawObject.addVertices(rectangle);
            //           
            //        }
            //        else {
            //            drawObject.addVertices(triangle);
            //        }

            //        drawObject.setModelTransormation(model);
            //        
            //    }
            //}
          
           

            //Framecounter frames;

    /*    while (!glfwWindowShouldClose(&renderer.device().getWindow())) {
            glfwPollEvents();
            renderer.draw();
            frames.next();
        }*/
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
