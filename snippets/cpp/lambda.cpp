#include <iostream>
#include <functional>
#include <array>
using namespace std;

template<typename Functor>
void doit_tmpl(Functor aFunctor)
{
    cout<<__FUNCTION__<<": "<<aFunctor(2, 3)<<endl;
}

void doit_func(function<int(int,int)> aFunction) 
{
    cout<<__FUNCTION__<<": "<<aFunction(2, 3)<<endl;
}

int main()
{
    //lamda as function argument
    auto f1 = [](int a, int b) -> int { return a + b;};
    doit_tmpl(f1);
    doit_func(f1);

    //lamba in lists
   function<int(int,int)> func_list[] = {
        [](int a, int b) { return a+b; },
        [](int a, int b) { return a-b; },
        [](int a, int b) { return a*b; },
        [](int a, int b) { return a/b; }, 
    };
    for(auto f: func_list) {
        cout<<f(5,1)<<endl;
    }





    return 0;
}