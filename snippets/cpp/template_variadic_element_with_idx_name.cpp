#include <iostream>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <string>
using namespace std;

template<typename T>
const char* type_to_string(const T& arg) {
    return typeid(arg).name();
}

template<typename... T, size_t... count, typename... K>
void helper(
    tuple<T...> args,
    integer_sequence<size_t,count...> sequ,
    std::tuple<K...> names
    ) 
{
    ((cout << count << "[" << std::get<count>(names)  << "]" << ":" << std::get<count>(args) << "; " ), ...);
}

template<typename... T>
void print_with_position(const T&... args) 
{
    auto names = std::make_tuple( type_to_string(args)... );

    helper(
        std::make_tuple(args...), 
        std::make_integer_sequence<size_t, sizeof...(args)>{},
        names
        );
}



int main()
{
    print_with_position("aaa", 1.234d, 0xABCDull);

  	return 0;
}