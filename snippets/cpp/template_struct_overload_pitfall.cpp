#include <string>
#include <vector>
#include <iostream>
#include <ranges>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <type_traits>
#include <type_traits>
using namespace std;


template<typename T>
struct my_t {
    static constexpr int value = 0;
};

template<typename T>
struct my_t<T*> {
    static constexpr int value = 1;
};

template<>
struct my_t<int*> {
    static constexpr int value = 2;
};


int main()
{
    cout << my_t<int>::value << endl;

    int* x = nullptr;
    cout << my_t<decltype(x)>::value << endl;

    float* y = nullptr;
    cout << my_t<decltype(y)>::value << endl;


    return 0;
}
