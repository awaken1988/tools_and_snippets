#include <algorithm>
#include <numeric>
#include <vector>
#include <iostream>
#include <type_traits>
#include <string>

using namespace std;

template<typename T>
void doit_impl(T t) {
    cout << t << endl;
}

template<typename T>
void doit(T t) {
    cout << "_last" << endl;
    doit_impl(t);
}

template<typename T, typename... E>
void doit(T t, E... e) {
    cout << "_next" << endl;
    doit_impl(t);
    doit(e...);
}




int main() {
 
    doit(1, 1.2, string{"asdf"});
  
    return 0;                            
}