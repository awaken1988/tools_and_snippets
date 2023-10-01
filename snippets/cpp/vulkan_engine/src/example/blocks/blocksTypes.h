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

            for (auto iField : ret) {
                ret.set(iField.pos, arg[iField.pos.x][iField.pos.y]);
            }

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
            auto ret = *this;
            
            x90 = x90 % 4;
            int rotationCount = x90 < 0 ? 4 - x90 : x90;

            for (int iRot = 0; iRot < rotationCount; iRot++) {
                Field<T, D> rotField{ {m_size.y, m_size.x} };
                for (const auto i : *this) {
                    std::cout << std::format("{} {} {}", i.pos.y, i.pos.x, i.get()) << std::endl;
                    rotField.set({ i.pos.y, m_size.x - 1 - i.pos.x }, i.get());
                }
                ret = rotField;
            }

            return ret;
        }

        struct tStartCount {
            int start = 0;
            int count = 0;
        };

        auto getFullRows() const {
            std::vector<tStartCount> ret;
            
            const auto xWidth = m_size.x;
            const auto xHeight = m_size.y;

            bool isOpen = false;

            for (int iY = 0; iY < xHeight; iY++) {
                bool isFilled = true;
                for (int iX = 0; iX < xWidth; iX++) {
                    if (get({ iX, iY }))
                        continue;
                    isFilled = false;
                    break;
                }
                
                if (isFilled) {
                    if (!isOpen) {
                        ret.emplace_back(tStartCount{ iY, 0 });
                        isOpen = true;
                    }
                    ret.back().count++;
                }
                else {
                    if (isOpen) {
                        isOpen = false;
                    }
                }
            }

            return ret;
        }

        //-------------------------------------------
        // iterator
        //-------------------------------------------
        struct iteratorSentinel {};

        struct IteratorItemConst {
        public:
            ivec2 pos;
            const Field* field;         

            IteratorItemConst()
                : pos{0,0} {}
            explicit IteratorItemConst(ivec2 pos, const Field* field)
                : pos{ pos }, field{ field } {}
            
            T get() const {
                return field->get(pos);
            }
        };

        struct IteratorItem : public IteratorItemConst {
        public:
            using IteratorItemConst::IteratorItemConst;

            void set(const T& value) {
                const_cast<Field*>(this->field)->set(this->pos, value);
            };
        };

        class IteratorConst {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = IteratorItemConst;
            using difference_type = long;
            using pointer = value_type*;
            using reference = value_type&;

            explicit IteratorConst(const Field& field)
                : m_field(field), m_pos{0,0} {}
            IteratorConst& operator++() {
                m_pos.x++;
                if (m_pos.x >= m_field.dimensions().x) {
                    m_pos.x = 0;
                    m_pos.y++;
                }
                return *this; 
            }
            IteratorConst operator++(int) {
                IteratorConst retval = *this; 
                ++(*this); 
                return retval; 
            }

            bool operator==(iteratorSentinel) const {
                return m_pos.y >= m_field.dimensions().y;
            }
            IteratorItemConst operator*() const {
                return IteratorItemConst( m_pos, &m_field );
            }
        protected:
            const Field& m_field;
            ivec2 m_pos;
        };

        class Iterator : public IteratorConst
        {
        public:
            using IteratorConst::IteratorConst;

            IteratorItem operator*() {
                return IteratorItem(this->m_pos, &this->m_field);
            }
        };

        auto begin() {
            return Iterator{ *this };
        }

        auto begin() const {
            return IteratorConst{ *this };
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
            //ret.m_pos += m_pivot;

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

        //TODO: make a view with worldPos,pos,data

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