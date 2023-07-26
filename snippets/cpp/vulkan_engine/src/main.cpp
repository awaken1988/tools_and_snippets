#include "render/vulkan/VulBase.h"
#include "render/vulkan/VulDevice.h"
#include "render/vulkan/VulRender.h"

#include "engine/EngineRender.h"

#include <string>
#include <memory>
#include <chrono>
#include <iostream>
#include <vector>

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
    using ivec2 = glm::ivec2;

    template<typename T, T D>
    class Field
    {
    public:
        Field() = default;
        Field(const Field&) = default;
        Field(ivec2 size) : 
            m_data{ std::vector<T>(static_cast<size_t>(size.x * size.y), D) },
            m_size{ size }
        {
            
        }

        ivec2 dimensions() const {
            return m_size;
        }

        bool get(ivec2 index) const {
            return m_data[indexOf(index)];
        }

        void set(ivec2 index, bool value) {
            m_data[indexOf(index)] = value;
        }

        auto rotate(int x90) const {
            Field<T,D> ret{ {m_size.y, m_size.x} };
            this->foreach([&](ivec2 index, bool value) {
                ret.set(index, value);
            });
            return ret;
        }

        template<typename F>
        void foreach(F f) const {
            for (int x = 0; x < m_size.x; x++) {
                for (int y = 0; y < m_size.y; y++) {
                    const ivec2 pos{ x,y };
                    f(pos, m_data[indexOf(pos)]);
                }
            }
        }
        template<typename F>
        void foreach(F f) {
            for (int x = 0; x < m_size.x; x++) {
                for (int y = 0; y < m_size.y; y++) {
                    const ivec2 pos{ x,y };
                    f(pos, m_data[pos]);
                }
            }
        }
    private:
        size_t indexOf(glm::ivec2 index) const {
            return index.y * m_size.x + index.x;
        }
    private:
        std::vector<T> m_data;
        ivec2 m_size;
    };

    using BoolField = Field<bool, false>;

    class Block
    {
    public:
        Block() = default;
        Block(const Block&) = default;
        Block(ivec2 pos, BoolField field)
            : m_pos{ pos }, m_field{ field }
        {}

        ivec2 lowerLeft() const {
            return m_pos;
        }
        ivec2 upperRight() const {
            return m_pos + m_field.dimensions();
        }
        int left() const {
            return m_pos.x;
        }
        int right() const {
            return m_pos.x + m_field.dimensions().x;
        }
        int top() const {
            return m_pos.y + m_field.dimensions().y;
        }
        int bottom() const {
            return m_pos.y;
        }
        Block move(ivec2 direction) const {
            return Block(m_pos + direction, m_field);
        }
        Block rotate(int x90) const {
            return Block(m_pos, m_field.rotate(x90));
        }
        bool checkCollision(const Block& other) const {
            const bool isX = engine::betweenStartEnd(left(), right(), other.left()) && engine::betweenStartEnd(left(), right(), other.right());
            const bool isY = engine::betweenStartEnd(bottom(), top(), other.bottom()) && engine::betweenStartEnd(bottom(), top(), other.top());
            if (!isX || isY)
                return false;

            const auto diff = ivec2{ other.left() - other.left(), other.bottom() - bottom() };
            const auto start = ivec2{
                diff.x < 0 ? 0 : diff.x,
                diff.y < 0 ? 0 : diff.y };
            const auto end = ivec2{
                1000 - std::abs(diff.x),
                1000 - std::abs(diff.y)};

            for (int iX = start.x; iX < end.x; iX++) {
                for (int iY = start.y; iY < end.y; iY++) {
                    const bool isSelf = m_field.get({ iX, iY });
                    const bool isOther = other.m_field.get({ iX, iY });

                    if (isSelf && isOther)
                        return true;
                }
            }

            return false;
        }
    private:
        BoolField m_field;
        ivec2 m_pos;
       
    };

    class GameState
    {
    private:
        struct tNextMove {
            int rotation;
            ivec2 direction;
            size_t blockIndex = std::numeric_limits<size_t>::max();
        };

    public:
        void input(size_t blockIndex, ivec2 nextDirection, int nextRotation) {
            if (m_next.has_value())
                return;
            m_next = tNextMove{};
            m_next->blockIndex = blockIndex;
            m_next->direction = nextDirection;
            m_next->rotation = nextRotation;
        }

        bool checkCollision(const Block& other) const {
            for (const auto& iOther : m_sticky) {
                if (other.checkCollision(iOther))
                    return true;
            }
            return false;

        }

        void updateMove() {
            if (!m_moving.has_value())
                return;

            bool isCollision = false;

            //first try custom move + rotation
            if (m_next.has_value()) {
                const auto movedBlock = (*m_moving)
                    .move(m_next->direction)
                    .rotate(m_next->rotation);
                isCollision = checkCollision(movedBlock);

                if (!isCollision)
                    m_moving = movedBlock;
            }

            //otherwise try downward
            if (isCollision) {
                const auto movedDownward = (*m_moving).move({ 0,-1 });
                isCollision = checkCollision(movedDownward);

                if (!isCollision)
                    m_moving = movedDownward;
            }

            if (!isCollision)
                return;
        }


    private:
        std::optional<Block> m_moving;
        std::vector<Block> m_sticky; // blocks not be movable anymore
        std::optional<tNextMove> m_next;
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
