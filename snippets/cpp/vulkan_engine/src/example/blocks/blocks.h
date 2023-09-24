#include "engine/EngineRender.h"

namespace blocks
{
    struct tSettings {
        std::chrono::microseconds updateInterval;
        glm::ivec2 tableSize;
    };

    void start(engine::Render& render, const tSettings& settings);
}