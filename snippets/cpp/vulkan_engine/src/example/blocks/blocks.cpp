#include "blocks.h"

#include "blocksTypes.h"

namespace blocks
{
    class GameState
    {
    private:
        struct tNextMove {
            int rotation = 0;
            ivec2 direction = {0,0};

            auto isMove() const {
                return !(rotation == 0 && direction == ivec2{0, 0});
            }
        };

    public:
        GameState(engine::Render& render) : 
            m_worldSize{10, 15},
            m_render{ render }, 
            m_world{ ivec2{0,0}, BoolField{m_worldSize} },
            m_drawField { m_worldSize }
        {
            auto rectangle = engine::primitive::rectanglePrimitive(); //TODO: fix addVertex...
            m_vertex = render.addVertex(rectangle);

            //init enough drawhandles from the render
            {
                const size_t drawing_elements = m_world.getField().size() + maxFigureBlocks();
                for (auto iHdnl = 0; iHdnl < drawing_elements; iHdnl++) {
                    auto drawHndl = m_render.addDrawable();
                    render.setVertex(drawHndl, m_vertex);
                    render.setEnabled(drawHndl, false);
                    m_drawMoving.push_back(drawHndl);
                }
            }

            //init world
            for(auto iCell: m_world.getField()) {
                if (iCell.pos.y < 3) {
                    iCell.set(1);
                }
            }

            //set camera
            {
                int width = 0, height = 0;
                glfwGetWindowSize(&render.window(), &width, &height);

                glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
                glm::mat4 proj = glm::perspective(glm::radians(34.0f), (float)width / (float)height, 0.1f, 60.0f);
                proj[1][1] *= -1;

                render.setViewProjection(view, proj);
            }
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

        void inputDirection(ivec2 direction) {
            m_next.direction = direction;
        }

        void inputRotation(int rotation) {
            m_next.rotation = rotation;
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

            m_next = tNextMove{};
        }

        void updateMove() {
            if (!m_moving.has_value())
                return;

            bool isCollision = false;

            //first try to rotate/move with the user input
            if (m_next.isMove()) {
                const auto movedBlock = (*m_moving)
                    .move(m_next.direction)
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
        }

        void updateDrawable() {
            //reset drawobjects -> non drawing
            for (size_t iHndl = 0; iHndl < m_drawMoving.size(); iHndl++) {
                m_render.setEnabled(m_drawMoving[iHndl], false);
            }

            size_t drawIndex = 0;

            //update world 
            for(auto iCell: m_world.getField()) {
                if (!iCell.get())
                    continue;

                const auto pos = m_world.getPos() + iCell.pos;

                auto draw = m_drawMoving[drawIndex];
                m_render.setWorldTransform(draw, toWorldTransform(pos));
                m_render.setEnabled(draw, true);

                drawIndex++;
            }

            //update moving object
            if (m_moving.has_value()) {
                for(auto iCell: m_moving->getField()) {
                    if (!iCell.get())
                        continue;

                    const auto pos =  m_moving->getPos() + iCell.pos;

                    auto draw = m_drawMoving[drawIndex];
                    m_render.setWorldTransform(draw, toWorldTransform(pos));
                    m_render.setEnabled(draw, true);

                    drawIndex++;
                }
            }
        }


    private:
        engine::Render& m_render;

        const ivec2 m_worldSize;

        Block m_world;
        std::optional<Block> m_moving;
        tNextMove m_next;
       
        Field<DrawableHandle, DrawableHandle{}> m_drawField;
        std::vector<DrawableHandle> m_drawMoving;

        VertexHandle m_vertex;
    };

    struct MoveDir {
        static constexpr ivec2 LEFT = { -1,0 };
        static constexpr ivec2 RIGHT = { -1,0 };
    };

    void start(engine::Render& render) {
        GameState gamestate{render};
        engine::RetryTimer timeout{ std::chrono::seconds{2} };
        
        auto&& window = &render.window();

        ivec2 direction{ 0,0 };
        int rotation = 0;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                gamestate.inputDirection(MoveDir::LEFT);
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                gamestate.inputDirection(MoveDir::RIGHT);
            }
            if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
                gamestate.inputRotation(-1);
            }
            if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
                gamestate.inputRotation(1);
            }

            if (timeout) {
                gamestate.update();
            }
            
            render.draw();
        }

    }
}