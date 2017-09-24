import sys
from antlr4 import *
from antlr_generated.demoLexer import demoLexer
from antlr_generated.demoParser import demoParser
from antlr_generated.demoListener import demoListener
 
class MyDemoListener(demoListener):

    def __init__(self, output):
        pass

    def enterAddition(self, ctx:demoParser.AdditionContext):
        print( ctx.getText() )
    
    def exitAddition(self, ctx:demoParser.AdditionContext):
        print( ctx.getText() )




def main(argv):
    input = FileStream(argv[1])
    lexer = demoLexer(input)
    stream = CommonTokenStream(lexer)
    parser = demoParser(stream)
    tree = parser.addition()
    
    output = open("testoutput.txt","w")
    
    myListener = MyDemoListener(output)
    walker = ParseTreeWalker()
    walker.walk(myListener, tree)
        
    output.close()      

    
if __name__ == '__main__':
    main(sys.argv)