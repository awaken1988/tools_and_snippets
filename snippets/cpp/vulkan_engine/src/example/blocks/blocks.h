#include "render/vulkan/VulBase.h"
#include "render/vulkan/VulDevice.h"
#include "render/vulkan/VulRender.h"

#include "engine/EngineRender.h"



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
            auto ret = Block(m_pos, m_field.rotate(x90));
            ret.m_pos += m_pivot;

            return ret;
        }
        bool containsX(int pos) const {
            return engine::betweenStartEnd(left(), right(), pos);
        }
        bool containsY(int pos) const {
            return engine::betweenStartEnd(bottom(), top(), pos);
        }
        bool checkCollision(const Block& other) const {
            const int leftPos = std::max(left(), other.left());
            const int rightPos = std::min( right(), other.right() );
            const bool isX = containsX(leftPos) && containsX(rightPos);
            
            const int bottomPos = std::max(bottom(), other.bottom());
            const int topPos = std::min(top(), other.top());
            const bool isY = containsY(bottomPos) && containsY(topPos);

            if ((!isX || !isY))
                return false;
            
            const int diffX = rightPos - leftPos;
            const int diffY = topPos - bottomPos;

            for (int iX = leftPos; iX < rightPos; iX++) {
                for (int iY = bottomPos; iY < topPos; iY++) {
                    const ivec2 selfOffset{ iX - m_pos.x, iY - m_pos.y };
                    const ivec2 otherOffset{ iX - m_pos.x, iY - m_pos.y };

                    if (m_field.get(selfOffset) && other.m_field.get(otherOffset))
                        return true;
                }
            }

            return false;
        }
    private:
        BoolField m_field;
        ivec2 m_pos;
        ivec2 m_pivot; 
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
            for (const auto& iOther : m_worlds) {
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
        std::vector<Block> m_worlds;
        std::optional<tNextMove> m_next;
    };
}