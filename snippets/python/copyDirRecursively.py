import glob
import os
import shutil
import filecmp

def copyRecursive(src, dest):
    os.chdir(src)
    changedFiles = []
   
    for i in glob.glob("*"):
        currDest = os.path.join(dest, i)
        if os.path.isfile(i):
            if not os.path.isfile(currDest) or not filecmp.cmp(i, currDest):
                    changedFiles.append(currDest)
            shutil.copyfile(i, currDest)
        if os.path.isdir(i):
            if not os.path.isdir(currDest):
                os.mkdir(currDest)
            changedFiles.extend(copyRecursive(i ,currDest))

    os.chdir("../")
    return changedFiles
                
print(copyRecursive(r"C:\Tcl", r"C:\tmp"))





    
