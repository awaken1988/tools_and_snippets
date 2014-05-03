'''
Created on 2 May 2014

@author: martin
'''
from __future__ import generators
import random
from tkinter import Tk, Canvas, Frame, BOTH, LEFT, Text, Label, Button, Y, X, BOTTOM,\
    Spinbox, SUNKEN, NW
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
        self._fileFrm.pack(padx=10, anchor=NW)
        self._fileLbl = Label(self._fileFrm, text="File:       ")
        self._fileLbl.pack(side=LEFT)
        self._fileTxt = Text(self._fileFrm, height=1, width=30)
        self._fileTxt.pack(side=LEFT, fill=X, expand=1)
        self._fileBtn = Button(self._fileFrm, text="Create", command=self._callbackFun)
        self._fileBtn.pack(side=LEFT)
    
    def _initSize(self):
        self._sizeFrm = Frame(self)
        self._sizeFrm.pack(padx=10, anchor=NW)
        self._sizeLbl = Label(self._sizeFrm, text="FileSize:  ")
        self._sizeLbl.pack(side=LEFT)
        self._sizeTxt = Text(self._sizeFrm, height=1, width=20)
        self._sizeTxt.pack(side=LEFT, fill=X, expand=1)
        
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



class GuiGenerateRandom(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack()
        
        self._lbl = Label(self, text="no parameters")
        self._lbl.pack()
        
    def getSettings(self):
        return {}
    
    def getName(self):
        return "Random"



class GuiGenerateCount(Frame):
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack()
        
        #step increment len
        self._stepLenFrm  = Frame(self); self._stepLenFrm.pack()
        self._stepLenLbl  = Label(self._stepLenFrm, text="Step Len: ");   self._stepLenLbl.pack(side=LEFT)
        self._stepLenSpin = Spinbox(self._stepLenFrm, from_=0, to=1000); self._stepLenSpin.pack(side=LEFT)
        
        #start value
        self._startFrm  = Frame(self)
        self._startLbl  = Label(self._startFrm, text="Start Value: ");   self._startLbl.pack(side=LEFT)
        self._startTxt  = Text(self._startFrm, width=20, height=1);   self._startTxt.pack(side=LEFT)
        
    def getSettings(self):
        return  {   "StepLen":      self._stepLenSpin.get(),
                    "StartValue":   self._startTxt.get()
                }
    
    def getName(self):
        return "Counter"
        
        
        
class GuiGeneratorSelect(Frame):
    def __init__(self, parent, generators):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack()
        
        self._generators = generators
        self._generatorName = StringVar()
        self._generatorName.set(generators[0])
        self._generatorName.trace("w", self._switchSettings)
        
        self._generatorLbl = Label(self, text="Generator");
        self._generatorLbl.pack(side=LEFT)
        
        param = (self, self._generatorName) + tuple(i.getName() for i in generators)
        self._generatorOpt = OptionMenu(*param)
        self._generatorOpt.pack(side=LEFT)
        
    def _switchSettings(self, *args):
       print("DBG: switch generator settings")
       for i in self._generators:
           if i.getName() == self._generatorName.get(): 
               i.pack()
               print("pack " + str(i.getName()))
           else:
               i.pack_forget() 
               print("unpack " + str(i.getName()))
                



class MainWindow(Frame):        
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack(fill=BOTH, expand=1)
        
        
        #basic settings
        self.basicSettings = GuiBasicSettings(self); self.basicSettings.pack()
        
        #separatr
        self._generatorSepFrm = Frame(self, bg="#000000", height=2)
        self._generatorSepFrm.pack(fill=X, expand=1, pady=20)
        
        # generator description
        genDescStr =                "Here you choose how the Binary file is generated\n"
        genDescStr = genDescStr +   " * Random:  creates an random image\n"
        genDescStr = genDescStr +   " * Counter: create a pattern like 112233 or 111122223333"
        
        self._generatorDescLbl = Label(self, text=genDescStr, justify=LEFT)
        self._generatorDescLbl.pack(ipadx=10)
        
        #generator select frames
        self._generatorSelectFrm = Frame(self)
        self._generatorSelectFrm.pack(pady=10)
        
        #generators
        self._generatorFrm = Frame(self, relief=SUNKEN, bd=2)
        self._generatorFrm.pack(fill=X, expand=1, padx=20)
        self.generatorFrames = ( GuiGenerateRandom(self._generatorFrm), 
                                 GuiGenerateCount(self._generatorFrm) 
                                )
        for i in self.generatorFrames:
            i.pack_forget()
            
        #generator select frames (self.generatorFrames)
        self_generatorSelect = GuiGeneratorSelect(self._generatorSelectFrm, self.generatorFrames)
    
        
        
        
        
root    = Tk()
ex      = MainWindow(root)
root.mainloop()
        








