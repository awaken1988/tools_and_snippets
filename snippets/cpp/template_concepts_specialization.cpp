#include <iostream>
#include <vector>
#include <list>
#include <memory>

namespace NsPtrTrait
{
    template<typename T>
    struct is_shptr {
        static constexpr bool value = false;
    };

    template<typename T>
    struct is_shptr<std::shared_ptr<T>> {
        static constexpr bool value = true;
    };


    template<typename T>
    struct is_ptr {
        static constexpr bool value = false;
    };

    template<typename T>
        requires (is_shptr<T>::value)
    struct is_ptr<T> {
        static constexpr bool value = true;
    };

 

    void example() {
        static_assert(is_ptr<std::shared_ptr<int>>::value, "bla");
        static_assert(!is_ptr<double>::value, "bla");
    }
}

int main()
{
    NsPtrTrait::example();

    return 0;
}