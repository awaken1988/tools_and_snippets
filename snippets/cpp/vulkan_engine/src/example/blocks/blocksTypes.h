#include "engine/EngineRender.h"

namespace blocks
{
    using DrawableHandle = engine::DrawableHandle;
    using VertexHandle = engine::VertexHandle;

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

        template<size_t X, size_t Y>
        static Field fromArray(T(&arg)[X][Y]) {
            Field ret{ {static_cast<int>(X), static_cast<int>(Y)} };

            ret.foreach([&](glm::ivec2 pos, auto&& data) {
                data = arg[pos.x][pos.y];
                });

            return ret;
        }

        ivec2 dimensions() const {
            return m_size;
        }

        size_t size() const {
            return m_size.x * m_size.y;
        }

        T get(ivec2 index) const {
            return m_data[indexOf(index)];
        }

        void set(ivec2 index, T value) {
            m_data[indexOf(index)] = value;
        }

        auto rotate(int x90) const {
            Field<T, D> ret{ {m_size.y, m_size.x} };
            this->foreach([&](ivec2 index, T value) {
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
                    f(pos, m_data[indexOf(pos)]);
                }
            }
        }

        struct iteratorItem {
        public:
            const ivec2 pos;
            Field* field;

            iteratorItem() 
                : pos{0,0} {}
            explicit iteratorItem(ivec2 pos, Field* field)
                : pos{ pos }, field{ field } {}
            void set(const T& value) {
                field->set(pos, value);
            };
            T get() const {
                return field->get(pos);
            }
        };

        struct iteratorSentinel{};

        class iterator {
        public:
            std::input_iterator_tag iterator_category;
            long value_type;
            long difference_type;
            T* pointer;
            iteratorItem reference;

            explicit iterator(Field& field) 
                : m_field(field), m_pos{0,0} {}
            explicit iterator(const Field& field) 
                : m_field(field), m_pos{0,0} {}
            iterator& operator++() { 
                m_pos.x++;
                if (m_pos.x >= m_field.dimensions().x) {
                    m_pos.x = 0;
                    m_pos.y++;
                }
                return *this; 
            }
            iterator operator++(int) { 
                iterator retval = *this; 
                ++(*this); 
                return retval; 
            }
            bool operator==(iteratorSentinel) const {
                return m_pos.y >= m_field.dimensions().y;
            }
            iteratorItem operator*() {
                return iteratorItem( m_pos, &m_field );
            }
        private:
            Field& m_field;
            ivec2 m_pos;
        };

        auto begin() {
            return iterator{ *this };
        }

        auto begin() const {
            return iterator{ *this };
        }

        auto end() const {
            return iteratorSentinel{};
        }

    private:
        size_t indexOf(glm::ivec2 index) const {
            return index.y * m_size.x + index.x;
        }
    private:
        std::vector<T> m_data;
        ivec2 m_size;
    };

    //Workaround with uint8_t becasue vector<bool> is something special
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
            const int rightPos = std::min(right(), other.right());
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
                    const ivec2 selffOffset{ iX - left(), iY - bottom() };
                    const ivec2 otherOffset{ iX - other.left(), iY - other.bottom() };

                    if (m_field.get(selffOffset) && other.m_field.get(otherOffset))
                        return true;
                }
            }

            return false;
        }

        const BoolField& getField() const {
            return m_field;
        }

        BoolField& getField() {
            return m_field;
        }

        const ivec2& getPos() const {
            return m_pos;
        }

    private:
        BoolField m_field;
        ivec2 m_pos;
        ivec2 m_pivot;
    };

    static const auto& getFigures() {
        constexpr auto O = false;
        constexpr auto I = true;

        static auto ret = std::invoke([&]() {
            std::vector<BoolField> ret;

            bool data[][2] = {
                {I,O},
                {I,O},
                {I,O},
                {I,I},
            };

            ret.push_back(BoolField::fromArray(data));

            return ret;
            });

        return ret;
    }

    size_t maxFigureBlocks() {
        size_t ret = 0;
        for (const auto& iFigure : getFigures()) {
            ret = std::max(ret, iFigure.size());
        }

        return ret;
    }

}