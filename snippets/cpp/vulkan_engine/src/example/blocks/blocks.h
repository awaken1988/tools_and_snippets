#include "engine/EngineRender.h"

namespace blocks
{
    struct tSettings {
        std::chrono::microseconds updateInterval;
        std::chrono::microseconds blockMoveInterval;
        glm::ivec2 tableSize;
    };

    void start(engine::Render& render, const tSettings& settings);
}