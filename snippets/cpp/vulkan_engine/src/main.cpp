#include "render/vulkan/VulBase.h"
#include "render/vulkan/VulDevice.h"
#include "render/vulkan/VulRender.h"

#include "engine/EngineRender.h"

#include <string>
#include <memory>
#include <chrono>
#include <iostream>

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



namespace primitive
{
    const auto l = -0.5f;
    const auto h = 0.5f;

    std::vector<engine::Vertex> trianglePrimitive()
    {
        return {
            {{-0.5f, -0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 1.0f},  {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 1.0f},   {1.0f, 1.0f, 1.0f}},
        };
    }

    std::vector<engine::Vertex> rectanglePrimitive()
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



namespace blocks
{
    struct Position
    {
        constexpr Position() = default;
        constexpr Position(int x, int y)
            : x{ x }, y{ x } {}

        int x = -1;
        int y = -1;

        Position operator+(const Position& other) {
            return Position{ x + other.x, y + other.y };
        }

    };

    constexpr int SIDE_LEN_MAX = 3;
    constexpr int FIELDS = SIDE_LEN_MAX * SIDE_LEN_MAX;
    constexpr Position FIELD_XY(SIDE_LEN_MAX ,SIDE_LEN_MAX );

    struct Block
    {
        Position pos;
        std::array<bool, FIELDS> fields; //!< solid=true; none=false

        Position lowerLeft() const {
            return pos;
        }

        Position upperRight() const {
            return lowerLeft() + FIELD_XY;
        }


        bool& getPropterty(size_t x, size_t y) {
            return fields[y * SIDE_LEN_MAX + x];
        }

        const bool getPropterty(size_t x, size_t y) const {
            return fields[y * SIDE_LEN_MAX + x];
        }

        int left() const {
            return pos.x;
        }

        int right() const {
            return pos.x + SIDE_LEN_MAX;
        }

        int top() const {
            return pos.y + SIDE_LEN_MAX;
        }

        int bottom() const {
            return pos.y;
        }

        void move(Position direction) {
            pos = pos + direction;
        }

        bool checkCollision(const Block& other) const {
            const bool isX = engine::betweenStartEnd(left(), right(), other.left()) && engine::betweenStartEnd(left(), right(), other.right());
            const bool isY = engine::betweenStartEnd(bottom(), top(), other.bottom()) && engine::betweenStartEnd(bottom(), top(), other.top());
            if (!isX || isY)
                return false;

            const auto diff = Position(other.left() - other.left(), other.bottom()-bottom());
            const auto start = Position(
                diff.x < 0 ? 0 : diff.x,
                diff.y < 0 ? 0 : diff.y);
            const auto end = Position(
                SIDE_LEN_MAX - std::abs(diff.x),
                SIDE_LEN_MAX - std::abs(diff.y));

            for (int iX = start.x; iX < end.x; iX++) {
                for (int iY = start.y; iY < end.y; iY++) {
                    const bool isSelf = getPropterty(iX, iY);
                    const bool isOther = other.getPropterty(iX, iY);

                    if (isSelf && isOther)
                        return true;

                }
            }
      
            return false;
        }
    };

    class GameState
    {
    public:


        void updateMove() {
            for (auto& iMoving : m_moving) {
                iMoving.move(Position(0, 1));

                const bool isCollision = std::invoke([&]() {
                    for (const auto& iOther : m_moving) {
                        if (&iMoving == &iOther)
                            continue;
                        if (iMoving.checkCollision(iOther))
                            return true;
                    }
                    return false;
                });


                
            }
        }


    private:
        std::vector<Block> m_moving; // blocks not yet touched the bottom of the playing field
        std::vector<Block> m_sticky; // blocks not be movable anymore
    };
}




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

            //while (!glfwWindowShouldClose(&renderer.device().getWindow())) {
            //    glfwPollEvents();
            //    renderer.draw();
            //    frames.next();
            //}
        }
        
    }
    catch(std::string& e) {
        std::cout << e << std::endl;
    }

    return 0;
}
