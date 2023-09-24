#include "render/vulkan/VulBase.h"
#include "render/vulkan/VulDevice.h"
#include "render/vulkan/VulRender.h"

#include "engine/EngineRender.h"

#include "example/blocks/blocks.h"


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


enum class AppType {
    BLOCKS_GAME,
    TRIANGLE_DEMO,
};

void app();

int main() 
{
    try {
        app();
    }
    catch (std::string& e) {
        std::cout << e << std::endl;
        return 1;
    }

    return 0;
}

void app() {
    const auto app = AppType::BLOCKS_GAME;

    using namespace engine;

    auto render = Render::createInstance(RenderBackend::VULKAN);

    switch (app) {
    case AppType::BLOCKS_GAME: {
        const blocks::tSettings settings {
            .updateInterval = 800ms,
            .tableSize = glm::ivec2{ 25, 20 },
        };
        blocks::start(*render, settings);
    } break;
    case AppType::TRIANGLE_DEMO: {
        using namespace engine;

        //blocks::start(*render);

        auto testPrimitiveTriangle = primitive::rectanglePrimitive();
        auto testPrimitiveRectangle = primitive::trianglePrimitive();

        auto rectangle = render->addVertex(testPrimitiveTriangle);
        auto triangle = render->addVertex(testPrimitiveRectangle);

        int width = 0, height = 0;
        glfwGetWindowSize(&render->window(), &width, &height);


        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 10; y++) {
                auto drawObject = render->addDrawable();
                //const auto swapChainExtents = render.device().swapChainExtent();
                //glm::mat4 model = glm::rotate(glm::mat4(1.0f), 1.0f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
                //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                //glm::mat4 proj = glm::perspective(glm::radians(45.0f), swapChainExtents.width / (float) swapChainExtents.height, 0.1f, 10.0f);

                glm::mat4 model = glm::translate(glm::mat4{10.f}, glm::vec3(10.0f - x * 2.1f, 10.0f - y * 2.1f, 0.0f));
                glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 40.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                glm::mat4 proj = glm::perspective(glm::radians(32.0f), width / (float)height, 0.1f, 60.0f);

                proj[1][1] *= -1;

                render->setViewProjection(view, proj);

                if ((x % 2) == 0 || (y % 2) == 0) {
                    render->setVertex(drawObject, rectangle);
                }
                else {
                    render->setVertex(drawObject, triangle);
                }

                render->setWorldTransform(drawObject, model);
            }
        }

        Framecounter frames;

        while (!glfwWindowShouldClose(&render->window())) {
            glfwPollEvents();
            render->draw();
            frames.next();
        }
    } break;
    }
}