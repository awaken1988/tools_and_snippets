#include <iostream>
#include <string>
#include <vector>
#include <memory>
using namespace std;

template <template <typename> class C, class E>
void test(C<E> l)
{
   cout << "function: " <<  sizeof(l) <<endl;
};

template <template <typename, typename> class C, class E, class F>
struct tTest
{
   using container_type = C<E,F>;
   using element_type   = E;
};

int main() 
{
  test(vector<int>());

  cout << "struct size: " << sizeof(tTest<vector, int, std::allocator<int>>::element_type) << endl;
  
  std::cout << "Hello from C++!";
  return 0;
}
