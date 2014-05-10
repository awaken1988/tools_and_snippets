'''
Created on 2 May 2014

@author: martin
'''
import random
from tkinter import Tk, Canvas, Frame, BOTH, LEFT, Text, Label, Button, Y, X, BOTTOM,\
    Spinbox, SUNKEN, NW, Entry
from tkinter import StringVar, OptionMenu



def generateRandom(aFile, aFileSize, aSpecificSettings): 
    random.seed()    
    for i in range(0, aFileSize):
        currVal = bytes([random.randrange(0, 256)])
        aFile.write(currVal)
        
def generateCounter(aFile, aFileSize, aSpecificSettings):
    aStart  = 0
    aIncLen = 0
    
    try:    aStart = int(aSpecificSettings["StartValue"])
    except: raise Exception("StartValue is invalid")
    
    try:    aIncLen = int(aSpecificSettings["StepLen"])
    except: raise Exception("IncLen is invalid")
    
    currIncLen = 0
    for i in range(0, aFileSize):
        if aStart > 255: aStart = 0;
        currVal = bytes([aStart])
        aFile.write(currVal)
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
        self._fileTxt = Entry(self._fileFrm)
        self._fileTxt.insert(0, "/tmp/out.txt")
        self._fileTxt.pack(side=LEFT, fill=X, expand=1)
        self._fileBtn = Button(self._fileFrm, text="Create", command=self._callbackFun)
        self._fileBtn.pack(side=LEFT)
    
    def _initSize(self):
        self._sizeFrm = Frame(self)
        self._sizeFrm.pack(padx=10, anchor=NW)
        self._sizeLbl = Label(self._sizeFrm, text="FileSize:  ")
        self._sizeLbl.pack(side=LEFT)
        self._sizeTxt = Entry(self._sizeFrm)
        self._sizeTxt.insert(0, "1024")
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
        mult = int(self.sizeUnits[self._sizeVar.get()])
        val  = int(self._sizeTxt.get())
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
    
    def getGeneratorFunction(self):
        return generateRandom;



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
        self._startFrm  = Frame(self); self._startFrm.pack()
        self._startLbl  = Label(self._startFrm, text="Start Value: ");   self._startLbl.pack(side=LEFT)
        self._startTxt  = Entry(self._startFrm);   self._startTxt.pack(side=LEFT)
        self._startTxt.insert(0, "0")
        
    def getSettings(self):
        return  {   "StepLen":      self._stepLenSpin.get(),
                    "StartValue":   self._startTxt.get()
                }
    
    def getName(self):
        return "Counter"
        
    def getGeneratorFunction(self):
        return generateCounter
        
        
class GuiGeneratorSelect(Frame):
    def __init__(self, parent, generators):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack()
        
        self._generators = generators
        self._generatorName = StringVar()
        self._generatorName.set(generators[0].getName())
        self._generatorName.trace("w", self._switchSettings)
        
        self._generatorLbl = Label(self, text="Generator");
        self._generatorLbl.pack(side=LEFT)
        
        param = (self, self._generatorName) + tuple(i.getName() for i in generators)
        self._generatorOpt = OptionMenu(*param)
        self._generatorOpt.pack(side=LEFT)
        
        
        self._switchSettings()
        
    def _switchSettings(self, *args):
       print("DBG: switch generator settings")
       for i in self._generators:
           if i.getName() == self._generatorName.get(): 
               i.pack()
               self._generatorGui = i
               print("pack " + str(i.getName()))
           else:
               i.pack_forget() 
               print("unpack " + str(i.getName()))
    
    def getCurrGeneratorGui(self):
        return self._generatorGui         
    
                



class MainWindow(Frame):        
    def __init__(self, parent):
        Frame.__init__(self, parent)
        self.parent = parent
        self.pack(fill=BOTH, expand=1)
        
        
        #basic settings
        self.basicSettings = GuiBasicSettings(self); self.basicSettings.pack()
        self.basicSettings.setCallback(self.generateFile)
        
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
        self._generatorSelect = GuiGeneratorSelect(self._generatorSelectFrm, self.generatorFrames)
    
        #error output
        self._infoFrm = Frame(self, relief=SUNKEN, bd=2)
        self._infoFrm.pack(fill=X, expand=1, pady=10) 
        self._infoLbl = Label(self, bg="#000000", text="nothing to do", fg="#00FF00")
        self._infoLbl.pack(fill=X, expand=1, padx=0)     
        self._infoColorNormal = "#00FF00"
        self._infoColorErr    = "#FF0000"
    
    def generateFile(self):
        print("generate")
        
        fileName        = self.basicSettings.getFileName()
        fileSize        = 0
        blockSize       = 1024**2
        currGeneratorGui    = self._generatorSelect.getCurrGeneratorGui()

        print(type(currGeneratorGui))
        currGenerator       = currGeneratorGui.getGeneratorFunction()
        specSettings        = currGeneratorGui.getSettings()
        
        
        #cast filesize
        try:
            fileSize = self.basicSettings.getFileSize()
            if fileSize < 1:
                raise Exception("")
        except:
            self._infoLbl.config(fg=self._infoColorErr, text="file size is invalid")
            return
        
        #open file
        try:
            currFile = open(fileName, "wb")
        except:
            self._infoLbl.config(fg=self._infoColorErr, text="cannot create file")
            return
        
        #write to file
        try:
           currGenerator(currFile, fileSize, specSettings);
        except Exception as exc:
            print(exc)
       
        
        
        
        
        
root    = Tk()
ex      = MainWindow(root)
root.mainloop()

a = """
file = open("/tmp/test.bin", "wb")

fileSize    = 2200
blockSize   = 512
buff        = b""
for i in generateCounter(fileSize, 0, 8 ):
    buff += i
    if len(buff) > blockSize:
        file.write(buff)
        buff = b""

if len(buff) > 0:
    file.write(buff)
"""




