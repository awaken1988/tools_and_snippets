using namespace std;

template<typename... T>
void endless(T... a) {
    endless(a..., 1);
}

int main()
{
    endless(1,2,3);
    return 0;
}