#include <algorithm>
#include <numeric>
#include <vector>
#include <iostream>
#include <type_traits>
#include <string>

using namespace std;

template<typename T, typename = enable_if_t<is_floating_point_v<T>>>
T doit() {
    cout << "T" << endl;
    return T{};
}

template<typename T, typename = enable_if_t<is_pointer_v<T>>>
T* doit() {
    cout << "T" << endl;
    return T{};
}


int main() {
 
    doit<float>();
  
    return 0;                            
}