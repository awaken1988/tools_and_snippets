import tkinter as tk
import json
import platform
import subprocess
import sys
import os


#TODO: umount before mount
#TODO: create symblink on Windows to Desktop
#TODO: linux ~/mnt/<name>
#TODO: windows ~/Desktop/<name>
#TODO: make other share types avail

if platform.system() == "Linux":    
    pass
elif platform.system() == "Windows":
    def execute_terminal(aCmd, aForeground):
        cmd = ["start"]
        if aForeground:
            cmd = ['start', '/wait', 'cmd', '/k']
        cmd = cmd + aCmd[:]
        print("execute_terminal: "+str(cmd))
        os.system(" ".join(cmd))

    def mount_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        cmd = ["net", "use", fixed_path]
        if "user" in aData:
            cmd.append("/user:"+aData["user"])
        execute_terminal(cmd, True)
        execute_terminal(["explorer.exe", fixed_path], False)

    def open_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        execute_terminal(["explorer.exe", fixed_path], False)

    def unmount_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        cmd = ["net", "use", fixed_path, "/delete"]
        execute_terminal(cmd, False)
        
else:
    raise Exception("Your Platform not yet supported")

def loadcfg(aCfg):
    f = open(aCfg, "r");
    c = f.read();
    return json.loads(c)

def useraction(aType, aData):
    print("useraction(\"\{}\", \"{}\")".format(str(aType), aData))

    if "mount" == aType:
        mount_share(aData)
    if "open" == aType:
        open_share(aData)
    if "unmount" == aType:
        unmount_share(aData)
        
window = tk.Tk()
window.title("mount drives")

#generate mount list
data = loadcfg("mount.json")
print(data)

iRow = 0
for iMount in data["mount"]:
    outer_lambda = lambda xType, xData: lambda: useraction(xType, xData)    

    share_lbl = tk.Label(window, text=iMount["name"])
    mnt_btn = tk.Button(window, text="mount", command=outer_lambda("mount", iMount) )
    open_btn = tk.Button(window, text="open", command=outer_lambda("open", iMount) )
    unmount_btn = tk.Button(window, text="unmount", command=outer_lambda("unmount", iMount) )
    
    share_lbl.grid(row=iRow, column=3)
    unmount_btn.grid(row=iRow, column=2)
    mnt_btn.grid(row=iRow, column=1)
    open_btn.grid(row=iRow, column=0)

    iRow = iRow + 1

window.mainloop()
