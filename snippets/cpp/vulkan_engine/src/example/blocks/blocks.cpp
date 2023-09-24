#include "blocks.h"

#include "blocksTypes.h"

using namespace std::chrono;

namespace blocks
{
    struct StateTimer {
        StateTimer() {
            resetTimer();
        }

        milliseconds timePassed() {
            return duration_cast<milliseconds>(steady_clock::now() - startTime);
        }

        void resetTimer() {
            startTime = steady_clock::now();
        }

        std::chrono::steady_clock::time_point startTime;
    };

    struct StateMoving : public StateTimer {
        int nextRotation;
        int nextLeftRight;
        bool isFastMode = false;
        Block movingBlock;

        StateMoving() {
            clearInput();
        }

        void clearInput() {
            nextRotation = 0;
            nextLeftRight = 0;
        }
    };

    struct StateIdle : public StateTimer {
        double progressPercent = 0.0;
    };

    using tStates = std::variant<StateMoving, StateIdle> ;

    class GameLogik
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
        GameLogik(const tSettings& settings) :
            m_worldSize{settings.tableSize},
            m_world{ ivec2{0,0}, BoolField{m_worldSize}},
            m_state{StateIdle{}},
            m_settings{settings}
        {
            //init world
            for(auto iCell: m_world.getField()) {
                if (iCell.pos.y < 3) {
                    iCell.set(1);
                }
            }
        }

        void inputDirection(int leftRight) {
            if (auto movingState = getMovingState(); movingState) {
                movingState->nextLeftRight = leftRight;
            }
        }

        void inputRotation(int rotation) {
            if (auto movingState = getMovingState(); movingState) {
                movingState->nextRotation = rotation;
            }
        }

        void inputFastMove() {
            if (auto movingState = getMovingState(); movingState) {
                movingState->isFastMode = true;
            }
        }

        bool checkCollision(const Block& other) const {
            return m_world.checkCollision(other);
        }

        void toIdleState() {
            m_state = StateIdle{};
        }

        void toMovingState() {
            StateMoving movingState;

            const glm::ivec2 startPos{ m_worldSize.x / 2, m_worldSize.y - 6 };
            movingState.movingBlock = Block{ startPos, getFigures()[0] };

            m_state = movingState;
        }

        void update() {
            if (auto state = std::get_if<StateIdle>(&m_state); state) {
                if (state->timePassed() > 800ms) {
                    toMovingState();
                }
            }
            else if (auto state = std::get_if<StateMoving>(&m_state); state) {
                if (state->timePassed() > m_settings.blockMoveInterval) {
                    updateMove();
                }
            }
        }

        void updateMove() {
            auto& movingState = std::get<StateMoving>(m_state);
            const auto& originalBlock = movingState.movingBlock;
            std::optional<Block> moved;

            //first try to rotate/move with the user input
            if (movingState.isFastMode) {
                const auto moveFast = originalBlock.move({ 0,-4 });
                const auto isCollision = checkCollision(moveFast);
                if (!isCollision)
                    moved = moveFast;
            }
            else {
                const auto movedBlock = originalBlock
                    .move(glm::ivec2{movingState.nextLeftRight, 0 })
                    .move({ 0,-1 }  )
                    .rotate(movingState.nextRotation);
                const auto isCollision = checkCollision(movedBlock);

                if (!isCollision)
                    moved = movedBlock;
            }

            // if all variants above have a collision simply go down 1
            if (!moved) {
                const auto movedDownward = originalBlock.move({ 0,-1 });
                const auto isCollision = checkCollision(movedDownward);
                if (!isCollision)
                    moved = movedDownward;
            }

            movingState.clearInput();
            movingState.resetTimer();

            // if there is no collision -> update
            if (moved) {
                movingState.movingBlock = *moved;
                return;
            }
                
            //copy to world
            for (auto iSrc: originalBlock.getField()) {
                if (!iSrc.get())
                    continue;

                const ivec2 worldPos = originalBlock.getPos() + iSrc.pos;
                m_world.getField().set(worldPos, iSrc.get());
            }

            toIdleState();
        }

        const Block& getWorld() const {
            return m_world;
        }

        const tStates& getState() const {
            return m_state;
        }

    private:
        StateMoving* getMovingState() {
            return std::get_if<StateMoving>(&m_state);
        }

    private:
        const ivec2 m_worldSize;

        tSettings m_settings;

        Block m_world;
        tStates m_state;
    };

    class GameRender
    {
    public:
        GameRender(engine::Render& render, GameLogik& gameLogik) 
            :   m_render{ render },
                m_gameLogik{ gameLogik }
        {
            auto rectangle = engine::primitive::rectanglePrimitive(); //TODO: fix addVertex...
            m_vertex = render.addVertex(rectangle);
        }

        void draw() {
            m_render.clearDrawable();

            const auto& world = m_gameLogik.getWorld();

            updateCamera();

            //update world 
            for (auto iCell : world.getField()) {
                if (!iCell.get())
                    continue;

                const auto pos = world.getPos() + iCell.pos;

                auto drawable = m_render.addDrawable();
                m_render.setWorldTransform(drawable, toWorldTransform(pos));
                m_render.setVertex(drawable, m_vertex);
                m_render.setEnabled(drawable, true);
            }

            //update moving object
            if (auto* state = std::get_if<StateMoving>(&m_gameLogik.getState()); state) {
                const auto& movingBlock = state->movingBlock;
                for (auto iCell : movingBlock.getField()) {
                    if (!iCell.get())
                        continue;

                    const auto pos = movingBlock.getPos() + iCell.pos;

                    auto drawable = m_render.addDrawable();
                    m_render.setWorldTransform(drawable, toWorldTransform(pos));
                    m_render.setVertex(drawable, m_vertex);
                    m_render.setEnabled(drawable, true);
                }
            }

            m_render.draw();
        }

        void updateCamera() {
            int width = 0, height = 0;
            glfwGetWindowSize(&m_render.window(), &width, &height);

            const double movingEye = 0.0;

            glm::mat4 view = glm::lookAt(glm::vec3(movingEye, movingEye, 60.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            glm::mat4 proj = glm::perspective(glm::radians(34.0f), (float)width / (float)height, 0.1f, 60.0f);
            proj[1][1] *= -1;

            m_render.setViewProjection(view, proj);
        }

        glm::mat4 toWorldTransform(glm::vec2 pos) {
            const float offset = 10.0f;
            const glm::vec3 translatePos{
                offset - (pos.x * 1.1f),
                    offset * 1.5f - (pos.y * 1.1f),
                    0.0f
            };

            return glm::translate(glm::mat4{ 1.0f }, translatePos);
        }

        glm::mat4 toWorldTransform(ivec2 pos) {
            return toWorldTransform(glm::vec2(pos.x, pos.y));
        }

    private:
        GameLogik& m_gameLogik;
        engine::Render& m_render;
        VertexHandle m_vertex;
    };

  
    void start(engine::Render& render, const tSettings& settings) {
        auto&& window = &render.window();
        
        GameLogik gamestate{settings};
        GameRender blocksRender{ render, gamestate };

        engine::KeyPressed keys{*window, 80ms};

        constexpr auto keyTimeout = 40ms;
       
        while (!glfwWindowShouldClose(window)) {
            glfwWaitEventsTimeout(0.01);

            if (keys.consumeKey(GLFW_KEY_A)) {
                gamestate.inputDirection(-1);
            }
            if (keys.consumeKey(GLFW_KEY_D)) {
                gamestate.inputDirection(1);
            }
            if (keys.consumeKey(GLFW_KEY_S)) {
                gamestate.inputFastMove();
            }
            if (keys.consumeKey(GLFW_KEY_Q)) {
                gamestate.inputRotation(-1);
            }
            if (keys.consumeKey(GLFW_KEY_E)) {
                gamestate.inputRotation(1);
            }

            gamestate.update();
            blocksRender.draw();
        }

    }
}