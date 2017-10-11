import sys
import inspect
from antlr4 import *
from build_py.demoLexer import demoLexer
from build_py.demoParser import demoParser
from build_py.demoListener import demoListener


class MyDemoVisitor(ParseTreeVisitor):

    def __init__(self, parser):
        self.parser = parser

    # Visit a parse tree produced by demoParser#init.
    def visitInit(self, ctx:demoParser.InitContext):
        for i in range(0, ctx.getChildCount()):
            curr = ctx.getChild(i)
            curr_text = curr.getText()
            
            if curr.getChildCount() > 0 and "InitContext" == type(curr.getChild(0)).__name__:
                self.visit(curr)
            elif "{" == curr.getText():
                print("[", end="")
            elif "}" == curr.getText():
                print("]", end="")
            elif "," == curr.getText():
                print(" ", end="")
            else:
                print(curr_text, end="")
            #{1,2,3,{4,5},6,7,8}
            #print(type(curr).__name__ + ": " + curr.getText() + ": ")
            #for k in range(0, curr.getChildCount()):
            #    sub_curr = curr.getChild(k)
            #    print("\t"+type(sub_curr).__name__)


    # Visit a parse tree produced by demoParser#value.
    def visitValue(self, ctx:demoParser.ValueContext):
        self.visit(ctx.getChild(0))

def main(argv):
    input = FileStream(argv[1])
    lexer = demoLexer(input)

    stream = CommonTokenStream(lexer)

    parser = demoParser(stream)
    tree = parser.outerinit()
    
    output = open("testoutput.txt","w")
    
    v = MyDemoVisitor(parser)
    v.visit(tree)
    output.close()      
    print("")
    
if __name__ == '__main__':
    main(sys.argv)