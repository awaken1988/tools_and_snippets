#include <iostream>
#include <memory>
using namespace std;

struct BigSizeStruct
{
    int x;
};

int main()
{


    for(int i=0; i<100000000; i++) {
        BigSizeStruct* p = new BigSizeStruct;
        shared_ptr<BigSizeStruct> pSmart(p);    
    }

    return 0;
}