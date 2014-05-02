'''
Created on 2 May 2014

@author: martin
'''
import random
from tkinter import Tk, Canvas, Frame, BOTH, LEFT, Text, Label, Button, Y, X, BOTTOM
from tkinter import StringVar, OptionMenu


def generateRandom(aStream, aSizeByte):
    random.seed()    
    for i in range(0, aSizeByte):
        currVal = bytes([random.randrange(0, 256)])
        aStream.write(currVal)
        
def generateCounter(aStream, aSizeByte, aStart, aIncLen):
    random.seed()
    
    currIncLen = 0
    for i in range(0, aSizeByte):
        if aStart > 255: aStart = 0;
        currVal = bytes([aStart])
        aStream.write(currVal)
        currIncLen = currIncLen + 1
        if currIncLen >= aIncLen:
            currIncLen = 0
            aStart     = aStart + 1;


            
###################################################
# Gui
###################################################
class GuiBasicSettings(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack()
        
        #Unit
        self.sizeUnits = {"Byte": 1, "KiB":1024, "MiB":1024**2, "GiB":1024**3}
        
        self._initFile()
        self._initSize()
    
    def _initFile(self):
        self._fileFrm = Frame(self)
        self._fileFrm.pack()
        self._fileLbl = Label(self._fileFrm, text="File: ")
        self._fileLbl.pack(side=LEFT)
        self._fileTxt = Text(self._fileFrm, height=1, width=30)
        self._fileTxt.pack(side=LEFT)
        self._fileBtn = Button(self._fileFrm, text="Create", command=self._callbackFun)
        self._fileBtn.pack(side=LEFT)
    
    def _initSize(self):
        self._sizeFrm = Frame(self)
        self._sizeFrm.pack(fill=X, expand=1)
        self._sizeLbl = Label(self._sizeFrm, text="FileSize")
        self._sizeLbl.pack(side=LEFT)
        self._sizeTxt = Text(self._sizeFrm, height=1, width=20)
        self._sizeTxt.pack(side=LEFT)
        
        self._sizeVar    = StringVar()
        self._sizeVar.set("Byte")       #FIXME: replace "Byte" with variable
        sizeOptParam = (self._sizeFrm, self._sizeVar) + tuple(self.sizeUnits.keys()) 
        self._sizeOptMen = OptionMenu(*sizeOptParam)
        self._sizeOptMen.pack(side=LEFT)
    
    def _callbackFun(self):
        print("_callbackBtn")
        self.outerCallback()
    
    def getFileName(self):
        return self._fileTxt.get()
        
    def getFileSize(self):
        mult = self.sizeUnits[self._sizeVar]
        val  = self._sizeTxt.get()
        return val * mult   
    
    def setCallback(self, aCallback):
        self.outerCallback = aCallback
        
    
        
        
    

class MainWindow(Frame):
            
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        
        self.parent.title = "ByteGenerator"
        self.pack(fill=BOTH, expand=1)
        
        self.generators         = ["random", "counter"] #Type of generatos
        self.generatorSelected  = StringVar()
        self.generatorSelected.set(self.generators[0])
        
        #filename
        self._initFilenme()
        
        #create freame for each generator
        self._initGenerators()
 
    def _initFilenme(self):
        self.filenameFrm    = Frame(self)
        self.filenameFrm.pack()
        self.filenameLbl    = Label(self.filenameFrm, text="Filename: ")
        self.filenameLbl.pack(side=LEFT)
        self.filenameTxt    = Text(self.filenameFrm, height=1, width=30)
        self.filenameTxt.pack(side=LEFT)
        
        #select a generator
        genArgs = (self.filenameFrm, self.generatorSelected) + tuple(self.generators)
        self.generatorOptMen = OptionMenu(*genArgs)
        self.generatorOptMen.pack(side=LEFT)
        self.generatorSelected.trace("w", self.changeGenerator)
        
        
        self.createBtn      = Button(self.filenameFrm, text="create", command=self.create)
        self.createBtn.pack(side=LEFT)
    
    def _initGenerators(self): 
        for i in self.generators:
            currFrame   = Frame(self)
            currFrame.pack(side=LEFT, expand=1, fill=BOTH)
            
            currLbl     = Label(currFrame, text=i)
            currLbl.pack(side=LEFT, expand=1, fill=BOTH)
            
    def create(self):
        print("create")
    
    def changeGenerator(self, *args):
        print("changeGenerator to={}".format(self.generatorSelected.get()))
        
        
        
        
root    = Tk()
ex      = GuiBasicSettings(root)
root.mainloop()
        








