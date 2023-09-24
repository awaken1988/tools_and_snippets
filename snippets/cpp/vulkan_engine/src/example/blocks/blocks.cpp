#include "blocks.h"

#include "blocksTypes.h"

namespace blocks
{
    class GameState
    {
    private:
        struct tNextMove {
            int rotation = 0;
            int leftRight = 0;
            bool isFast = false;

            auto isMove() const {
                return !(rotation == 0 && leftRight == 0);
            }
        };

    public:
        GameState(engine::Render& render, const tSettings& settings) :
            m_worldSize{settings.tableSize},
            m_render{ render }, 
            m_world{ ivec2{0,0}, BoolField{m_worldSize} }
        {
            auto rectangle = engine::primitive::rectanglePrimitive(); //TODO: fix addVertex...
            m_vertex = render.addVertex(rectangle);

            //init world
            for(auto iCell: m_world.getField()) {
                if (iCell.pos.y < 3) {
                    iCell.set(1);
                }
            }

            //set camera
            updateCamera();
        }

        void updateCamera() {
            int width = 0, height = 0;
            glfwGetWindowSize(&m_render.window(), &width, &height);

            const double movingEye = m_camera_animation.counter;

            glm::mat4 view = glm::lookAt(glm::vec3(movingEye, movingEye, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            glm::mat4 proj = glm::perspective(glm::radians(34.0f), (float)width / (float)height, 0.1f, 60.0f);
            proj[1][1] *= -1;

            m_render.setViewProjection(view, proj);

            //update animation
            //m_camera_animation.counter += m_camera_animation.countUp ?
            //    m_camera_animation.step : -m_camera_animation.step;
            //if (std::abs(m_camera_animation.counter) > m_camera_animation.maxValue)
            //    m_camera_animation.countUp = !m_camera_animation.countUp;
        }

        glm::mat4 toWorldTransform(glm::vec2 pos) {
            const float offset = 10.0f;
            const glm::vec3 translatePos {
                offset - (pos.x * 1.1f),
                    offset * 1.5f - (pos.y * 1.1f),
                    0.0f
            };

            return glm::translate(glm::mat4{1.0f}, translatePos);
        }

        glm::mat4 toWorldTransform(ivec2 pos) {
            return toWorldTransform(glm::vec2(pos.x, pos.y));
        }

        void inputDirection(int leftRight) {
            m_next.leftRight += leftRight ;
        }

        void inputRotation(int rotation) {
            m_next.rotation = rotation;
        }

        void inputFastMove() {
            m_next.isFast = true;
        }

        bool checkCollision(const Block& other) const {
            return m_world.checkCollision(other);
        }

        void update() {
            if (!m_moving.has_value()) {
                const glm::ivec2 startPos {m_worldSize.x/2, m_worldSize.y-6};
                m_moving = Block{ startPos, getFigures()[0] };
            }
            
            updateMove();
            updateDrawable();
            updateCamera();

            m_next = tNextMove{};
        }

        void updateMove() {
            if (!m_moving.has_value())
                return;

            bool isCollision = false;

            //first try to rotate/move with the user input
            if (m_next.isMove()) {
                const auto movedBlock = (*m_moving)
                    .move(glm::ivec2{m_next.leftRight, 0 })
                    .move({ 0,-1 }  )
                    .rotate(m_next.rotation);
                isCollision = checkCollision(movedBlock);

                if (!isCollision)
                    m_moving = movedBlock;
            }

            //otherwise simply go downward
            if (!m_next.isMove() || isCollision) {
                const auto movedDownward = (*m_moving).move({ 0,-1 });
                isCollision = checkCollision(movedDownward);

                if (!isCollision)
                    m_moving = movedDownward;
            }

            if (!isCollision)
                return;

            //copy to world
            for (auto iSrc: m_moving->getField()) {
                if (!iSrc.get())
                    continue;

                const ivec2 worldPos = m_moving->getPos() + iSrc.pos;
                m_world.getField().set(worldPos, iSrc.get());
            }
            m_moving.reset();

        }

        void updateDrawable() {
            m_render.clearDrawable();

            //update world 
            for(auto iCell: m_world.getField()) {
                if (!iCell.get())
                    continue;

                const auto pos = m_world.getPos() + iCell.pos;

                auto drawable = m_render.addDrawable();
                m_render.setWorldTransform(drawable, toWorldTransform(pos));
                m_render.setVertex(drawable, m_vertex);
                m_render.setEnabled(drawable, true);
            }

            //update moving object
            if (m_moving.has_value()) {
                for(auto iCell: m_moving->getField()) {
                    if (!iCell.get())
                        continue;

                    const auto pos =  m_moving->getPos() + iCell.pos;

                    auto drawable = m_render.addDrawable();
                    m_render.setWorldTransform(drawable, toWorldTransform(pos));
                    m_render.setVertex(drawable, m_vertex);
                    m_render.setEnabled(drawable, true);
                }
            }
        }


    private:
        engine::Render& m_render;

        const ivec2 m_worldSize;

        Block m_world;
        std::optional<Block> m_moving;
        tNextMove m_next;
       
        VertexHandle m_vertex;

        struct {
            double counter = 0.0;
            const double step = 0.8;
            const double maxValue = 35.0;
            bool countUp = true;
        } m_camera_animation;
    };

    struct MoveDir {
        static constexpr ivec2 LEFT = { -1,0 };
        static constexpr ivec2 RIGHT = { -1,0 };
    };

    void start(engine::Render& render, const tSettings& settings) {
        GameState gamestate{render, settings};
        
        engine::RetryTimer updateTimer{ settings.updateInterval };
        
        auto&& window = &render.window();

        ivec2 direction{ 0,0 };
        int rotation = 0;

        while (!glfwWindowShouldClose(window)) {
            const auto timeoutSec = std::chrono::duration_cast<std::chrono::duration<double>>(settings.updateInterval).count();
            glfwWaitEventsTimeout(timeoutSec);

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                gamestate.inputDirection(-1);
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                gamestate.inputDirection(1);
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                gamestate.inputFastMove();
            }
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                gamestate.inputRotation(-1);
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                gamestate.inputRotation(1);
            }

            if (updateTimer) {
                gamestate.update();
            }
            
            render.draw();
        }

    }
}