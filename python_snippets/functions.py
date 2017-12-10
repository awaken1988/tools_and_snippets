

def addNumber(num1, num2, text="no text"):
    print("{} + {} = {} #text={}".format(num1, num2, num1+num2, text))

addNumber(1, 2)
addNumber(1, num2=3)
addNumber(num1=2, num2=4)
addNumber(3, 6, "blaa")

addNumber( *[1,2] )
addNumber( *(1,2) )

addNumber( **{'num1': 8, 'num2': 60})




def mulNumber(mult1, *multOthers):
    result = mult1;
    for i in multOthers:
        result *= i;

    print(result);

mulNumber(1)
mulNumber(1, 2, 3)
mulNumber(1, *[8, 9])
