template<typename T>
struct tInner {
  T inner;
};

template<typename T, template<typename> typename C>
struct tMy {
	C<T> bla;
};

int main()
{
	tMy<int, tInner> x;
  
  	return 0;
}