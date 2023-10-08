#include "blocks.h"

#include "blocksTypes.h"

using namespace std::chrono;
using namespace std::ranges;

namespace blocks
{
    //--------------------------------------------------
    // States
    //--------------------------------------------------

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

    struct StateClearBlocks : public StateTimer {
        std::vector<tStartCount> blockRange;
        int columnPos = 0;  //current column pos which disappear
    };

    struct StateIdle : public StateTimer {
       
    };

    using tStates = std::variant<StateMoving, StateIdle, StateClearBlocks> ;



    //--------------------------------------------------
    // Logik   
    //--------------------------------------------------
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
            const bool isLeft = other.left() < 0;
            const bool isRight = other.right() > m_worldSize.x;
            const bool isBottom = other.bottom() < 0;

            return isLeft || isRight || isBottom || m_world.checkCollision(other);
        }

        void toIdleState() {
            m_state = StateIdle{};
        }

        void toStateClearBlocks(const std::vector<tStartCount>& blockRange ) {
            m_state = StateClearBlocks{
                .blockRange = blockRange,
                .columnPos = 0,
            };
        }

        void toMovingState() {
            StateMoving movingState;

            const glm::ivec2 startPos{ m_worldSize.x / 2, m_worldSize.y };
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
            else if (auto state = std::get_if<StateClearBlocks>(&m_state); state) {
                updateStateClearBlocks();
            }
        }

        void updateMove() {
            auto& movingState = std::get<StateMoving>(m_state);
            const auto& originalBlock = movingState.movingBlock;
            std::optional<Block> moved;

            //first try to rotate/move with the user input
            if (movingState.isFastMode) {
                const auto moveFast = originalBlock.move({ 0,-4 });
                if (!checkCollision(moveFast))
                    moved = moveFast;
            }
            else {
                const auto movedBlock = originalBlock
                    .move(glm::ivec2{movingState.nextLeftRight, 0 })
                    .move({ 0,-1 }  )
                    .rotate(movingState.nextRotation);
                if (!checkCollision(movedBlock))
                    moved = movedBlock;
            }

            // if all variants above have a collision simply go down 1
            if (!moved) {
                const auto movedDownward = originalBlock.move({ 0,-1 });
                if (!checkCollision(movedDownward))
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

            //find full rows
            const auto fullRows = m_world.getField().getFullRows();

            if (fullRows.empty())
                toIdleState();
            else
                toStateClearBlocks(fullRows);
        }

        void updateStateClearBlocks()
        {
            auto& state = std::get<StateClearBlocks>(m_state);
            auto& blockRange = state.blockRange;
            auto& columnPos = state.columnPos;

            if (state.timePassed() < m_settings.blocksClearDelay)
                return;
            if (state.columnPos > m_worldSize.x) {
                auto& currRange = blockRange.back();
                
                collapseRow(currRange.last());
                columnPos = 0;

                if (currRange.count == 1) {
                    blockRange.pop_back();
                }
                else {
                    currRange.count--;
                }
                if (blockRange.empty()) {
                    toIdleState();
                    return;
                }
            }  
          
            m_world.getField().set({ columnPos, blockRange.back().last()}, false);
            columnPos++;
            state.resetTimer();
        }

        void collapseRow(int posY) {
            for (const auto y : views::iota(posY, m_worldSize.y)) {
                for (const auto x : views::iota(0, m_worldSize.x)) {
                    const bool value = (y == m_worldSize.y - 1) ? false : m_world.getField().get({ x, y + 1 });
                    m_world.getField().set({ x, y }, value);
                }
            }
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

    //--------------------------------------------------
    // Render   
    //--------------------------------------------------
    class GameRender
    {
    public:
        static std::vector<engine::Vertex> createRectangle(glm::vec3 color)
        {
            using namespace engine::primitive;

            return {
                 {{l, l, 1.0f}, color},
                 {{h, l, 1.0f}, color},
                 {{h, h, 1.0f}, color},

                 {{h, h, 1.0f}, color},
                 {{l, h, 1.0f}, color},
                 {{l, l, 1.0f}, color},
            };
        }


        GameRender(engine::Render& render, GameLogik& gameLogik) 
            :   m_render{ render },
                m_gameLogik{ gameLogik }
        {
            auto redRectangle = createRectangle({1.0f, 0.0f, 0.0f});
            m_figureRectangle = render.addVertex(redRectangle);

            auto greyRectangle = createRectangle({ 0.1f, 0.1f, 0.1f });
            m_levelBoundary = render.addVertex(greyRectangle);
        }

        void drawRectangle(const glm::ivec2& pos, const VertexHandle& vertex) {
            auto drawable = m_render.addDrawable();
            m_render.setWorldTransform(drawable, toWorldTransform(pos));
            m_render.setVertex(drawable, vertex);
            m_render.setEnabled(drawable, true);
        }

        void draw() {
            m_render.clearDrawable();

            const auto& world = m_gameLogik.getWorld();
            const auto& field = world.getField();

            updateCamera();

            //draw world boundary
            for (int iX = 0; iX < field.dimensions().x; iX++) {
                drawRectangle({ iX, -1 }, m_levelBoundary);
            }
            for (const auto iX : { -1, field.dimensions().x }) {
                for (int iY = 0; iY < field.dimensions().y; iY++) {
                    drawRectangle({ iX, iY }, m_levelBoundary);
                }
            }
           
            //draw world figures
            for (auto iCell : world.getField()) {
                if (!iCell.get())
                    continue;
                const auto pos = world.getPos() + iCell.pos;
                drawRectangle(pos, m_figureRectangle);
            }

            //update moving object
            if (auto* state = std::get_if<StateMoving>(&m_gameLogik.getState()); state) {
                const auto& movingBlock = state->movingBlock;
                for (auto iCell : movingBlock.getField()) {
                    if (!iCell.get())
                        continue;
                    const auto pos = movingBlock.getPos() + iCell.pos;
                    drawRectangle(pos, m_figureRectangle);
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
        VertexHandle m_figureRectangle;
        VertexHandle m_levelBoundary;
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