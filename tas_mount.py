#Reference:
#   - https://ss64.com/nt/
#   - https://docs.microsoft.com/de-de/powershell/module/Microsoft.PowerShell.Core/About/about_PowerShell_exe?view=powershell-5.1

#TODO: umount before mount
#TODO: create symblink on Windows to Desktop
#TODO: linux ~/mnt/<name>
#TODO: windows ~/Desktop/<name>
#TODO: make other share types avail

import tkinter as tk
import json
import platform
import subprocess
import sys
import os

IS_LINUX = platform.system() == "Linux"
IS_WINDOWS = platform.system() == "Windows"

if IS_LINUX:    
    pass
elif IS_WINDOWS:
    def execute_terminal(aCmd, aForeground, aBlock=True):
        cmd = ['start', ]
        if aBlock:
            cmd.append("/wait")
        cmd.append('powershell.exe')
        if aForeground:
            cmd.append('-NoExit')
        cmd.append("-Command")
        cmd = cmd + aCmd[:]
        print("execute_terminal: "+str(cmd))
        subprocess.call(cmd, shell=True)

class ServiceMount:
    @staticmethod
    def getActions(aEntry):
        if IS_WINDOWS:
            return (
                {"name": "mount",   "action": ServiceMount.win_mount_share},
                {"name": "open",    "action": ServiceMount.win_open_share},
                {"name": "umount",  "action": ServiceMount.win_unmount_share},
            )
        return None

    @staticmethod
    def getDisplay(aEntry):
        return ("{}").format(aEntry["share"])

    @staticmethod
    def win_mount_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        cmd = ["net", "use", fixed_path]
        if "user" in aData:
            cmd.append("/user:"+aData["user"])
        execute_terminal(cmd, True)
        execute_terminal(["explorer.exe", fixed_path], False)

    @staticmethod
    def win_open_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        execute_terminal(["explorer.exe", fixed_path], False)

    @staticmethod
    def win_unmount_share(aData):
        fixed_path = aData["share"].replace('/', "\\")
        cmd = ["net", "use", fixed_path, "/delete"]
        execute_terminal(cmd, True)

class ServiceSsh:
    @staticmethod
    def getActions(aEntry):
        if IS_WINDOWS:
            return ({"name": "connect",   "action": ServiceSsh.win_connect},)
        return None

    @staticmethod
    def getDisplay(aEntry):
        return ("{} {}").format(aEntry["name"], aEntry["address"])


    @staticmethod
    def win_connect(aData):
        cmd = ["powershell.exe", "ssh", "{}@{}".format(aData["user"], aData["address"])]
        if "port" in aData: cmd += ["-p", str(aData["port"])]
        execute_terminal(cmd, False, False)
        
def loadcfg(aCfg):
    f = open(aCfg, "r")
    c = f.read()
    return json.loads(c)

def useraction(aType, aData):
    print("useraction(\"\{}\", \"{}\")".format(str(aType), aData))

    if "mount" == aType:
        mount_share(aData)
    if "open" == aType:
        open_share(aData)
    if "unmount" == aType:
        unmount_share(aData)

services = {
    "smb": ServiceMount,
    "ssh": ServiceSsh,
}



window = tk.Tk()
window.title("mount drives")

#generate mount list
data = loadcfg(sys.argv[1])
print(data)

iRow = 1

for iEntry in data["mount"]:
    iType = iEntry["type"]
    if iType not in services:
        print("WRN: ignore unkown entry {}".format(str(iEntry)))
        continue
    
    iService = services[iType]

    iCol = 0

    #type Label
    tk.Label(window, text=iEntry["type"]+": ").grid(row=iRow, column=iCol, sticky=tk.E)
    iCol += 1
    
    #name Label
    tk.Label(window, text=iEntry["name"]).grid(row=iRow, column=iCol, sticky=tk.W)
    iCol += 1

    #service specific text
    tk.Label(window, text=iService.getDisplay(iEntry)).grid(row=iRow, column=iCol, sticky=tk.W)
    iCol += 1

    #buttons
    for iAction in iService.getActions(iEntry):
        outer_lambda = lambda aAction,aEntry: lambda: aAction["action"](aEntry)    
        btn = tk.Button(window, text=iAction["name"], command=outer_lambda(iAction, iEntry))

        btn.grid(row=iRow, column=iCol)
        iCol += 1

    iRow = iRow + 1

window.mainloop()
