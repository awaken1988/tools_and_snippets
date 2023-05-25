#include <iostream>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <tuple>
using namespace std;

template<typename... T, size_t... count>
void helper(
    tuple<T...> args,
    integer_sequence<size_t,count...> sequ) 
{
    ((cout << count << ":" << std::get<count>(args) << "; " ), ...);
}

template<typename... T>
void print_with_position(const T&... args) 
{
    helper(std::make_tuple(args...), std::make_integer_sequence<size_t, sizeof...(args)>{});
}



int main()
{
    print_with_position("a", "b", "c");

  	return 0;
}