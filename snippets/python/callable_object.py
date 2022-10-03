#http://www.diveintopython3.net/special-method-names.html

class TestClass:
    def doit(self):
        self.testvar=(2, 4, 6, 8, 10)

    def __call__(self, x):
        return x*x

    def __str__(self):
        return "blaaa"

    def __iter__(self):
        self.index = -1;

    def __next(self):
        self.index = self.index + 1;
        return self.testvar[self.index]

    #interesting
    #def __getattribute__(self, x):
    #    return "fooo"
        

a = TestClass();
a.doit()


#callable objects
print("__call__: "+str(a(3)))

#string represantion
print("__str__: "+str(a))

#iterate object         T-O-D-O
#for i in a:
#    print(i)

#manipulating attributes
print(a.testvar)

