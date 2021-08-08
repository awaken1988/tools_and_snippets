#include <iostream>
#include <string>
#include <type_traits>
using namespace std;


template<typename T, enable_if_t<is_integral_v<T>, bool> = true>
void print(T a) {
    cout << "[integral type] " << a << endl;
}

template<typename T, enable_if_t<is_same_v<T, string>, bool> = true>
void print(T a) {
    cout << "[string type]" << a << endl;
}

template<size_t SIZE>
void print(const char(&a)[SIZE]) {
    cout << " [raw string] " << a << endl;
}

template<typename T, size_t SIZE>
void print(T(&a)[SIZE]) {
    cout << "[array] {" << endl;
    for (size_t i = 0; i < SIZE; i++) {
        print(a[i]);
    }
    cout << "}";
    
}


int main()
{
    print(1);
    print(string("asdf"));
    print("asdf");

    int a[] = { 1,2,3 };
    print(a);

    return 0;
}