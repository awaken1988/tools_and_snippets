#include <iostream>
#include <string>
#include <type_traits>
using namespace std;


template<typename T>
struct Trait {};

template<>
struct Trait<float> {
    static const bool is_func = true;
    static string toString(float) {
        return "float";
    }
    static const int num = 0;

    using type = float;
};

template<>
struct Trait<string> {
    static const bool no_func = true;
    static const int num = 1;

    using type = string;
};

template<
    typename T,
    typename T2 = void
>
struct tHelper {
    static string helper(typename T::type v) {
        return Trait<T::type>::toString(v);
    }
};


template<typename T>
struct tHelper<T, typename enable_if_t<T::no_func>> {
    static string helper(typename T::type v) {
        return "string";
    }
};



int main()
{
    cout << tHelper<Trait<float>>::helper(1.2f) << endl;
    cout << tHelper<Trait<string>>::helper(string("dffdff")) << endl;

    return 0;
}