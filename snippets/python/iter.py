
class Fibonacci:
    def __init__(self, max):
        self.max = max;

    def __iter__(self):
        self.a = 1
        self.b = 1;
        return self

    def __next__(self):
        ret = self.a

        self.a = self.b
        self.b = ret + self.b

        if( self.a > self.max ):
            raise StopIteration;

        return self.a;

for i in Fibonacci(1000):
    print(i)
