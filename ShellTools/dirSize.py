import os
import pprint
import sys
import getopt


#References
#    - https://wiki.python.org/moin/HowTo/Sorting
#    - http://pymotw.com/2/getopt/
#
#

def createItemTree(aPath):
    if not os.path.isdir(aPath):    raise Exception("path is not valid")
    
    vItems =  {"dir": [], "file": []}
    
    for i in os.listdir(aPath):
        vFullPath = os.path.join(aPath, i)
        if os.path.isfile(vFullPath):   
            vItems["file"].append({ "name":vFullPath, 
                                    "size":os.path.getsize(vFullPath)})
        if os.path.isdir(vFullPath):    
            vItems["dir"].append({  "name":     vFullPath,
                                    "content":  createItemTree(vFullPath)
                             })
    return vItems 

def addDirSize(aItemTree):
    vByteSize = 0
    
    for i in aItemTree["file"]:
        vByteSize += i["size"]
    for i in aItemTree["dir"]:
        i["size"] = addDirSize(i["content"])
        vByteSize += i["size"]
    
    return vByteSize;

def truncateString(aStr, aMaxLen):
    if len(aStr) > aMaxLen:
        return aStr[:aMaxLen-3]+"..."
    else:
        return aStr
    
def printItemTree(aItemTree, **aOpt):
   if "maxdepth" not in aOpt:        aOpt["maxdepth"] = 999999
   if "depth" not in aOpt:           aOpt["depth"] = 0
   if "nofiles" not in aOpt:         aOpt["withoutfiles"] = False
   
   if aOpt["depth"] >= aOpt["maxdepth"]:
        return

   sortedFiles  = sorted(aItemTree["file"], key=lambda item: item["size"], reverse=True)
   sortedDir    = sorted(aItemTree["dir"],  key=lambda item: item["size"], reverse=True)
    
   pre = " "*(aOpt["depth"] * 4)
   if not aOpt["nofiles"]:
       for i in sortedFiles:
           vFileName = truncateString(i["name"], 60)
           print(pre + "* [{1:10f}]  {0:s} ".format(vFileName, i["size"] / 1024 ** 2))
   for i in sortedDir:
       vFileName = truncateString(i["name"], 60)
       print(pre + "+ [{1:10f}]  {0:s}".format(vFileName, i["size"] / 1024 ** 2))
       vNext = dict(aOpt)
       vNext["depth"] += 1; 
       printItemTree(i["content"], **vNext)
    

options, remainder = getopt.getopt(sys.argv[1:], "d:n")
vMaxDepth = 999999
vNoFiles = False

for opt, arg in options:
    if(opt in "-d"):
        vMaxDepth=int(arg)
    if(opt in "-n"):
        vNoFiles = True;



dirTree = createItemTree(remainder[0]);
addDirSize(dirTree)
printItemTree(dirTree, maxdepth=vMaxDepth, nofiles=vNoFiles);

