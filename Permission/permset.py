import os.path
import json
import subprocess
import sys
import os

def get_script_path():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

def sub_path_list(aPath):
    splitted = os.path.split(aPath);
    drive = os.path.split(os.path.splitdrive(aPath)[1])
    if( len(splitted[0]) < 1 ):
        return [];
    return [splitted[0]] + sub_path_list(splitted[0])

def sub_path_isdir(aPathList):
    for i in aPathList:
        if not os.path.isdir(i):
            print("ERROR: path doesn't exist '{}'".format(i) )

def read_cfg(aCfgPath):
    print("Read json config: \""+aCfgPath+"\"")
    file = open(aCfgPath, "r")
    return json.loads(file.read())

def executeCmd(aCommand):
    vRet = subprocess.call(aCommand, shell=True)
    if vRet != 0:
        raise Exception(aCommand)

def setacl(aSubCfg):
    for iEntry in aSubCfg:
        #create dir list
        dirs = []
        for iPath in iEntry["path"]:
            dirs = dirs + [(iPath, True)]
            for iSubDir in sub_path_list(iPath):
                dirs = dirs + [(iSubDir, False)]

        #create acl
        for iAcl in iEntry["acl"]:
            for iDir in dirs:
                #print("SETACL acl={}; recursive={} dir={}".format(iAcl, int(iDir[1]), iDir[0]) )
                cmd = "setfacl "
                if( iDir[1] ):
                    cmd += " -R "
                else:
                    cmd += "    "
                cmd += " -m "
                cmd += iAcl + " " + iDir[0]
                print(cmd)
                executeCmd(cmd)



if len(sys.argv) > 1 and sys.argv[1].endswith("json"):
    os.chdir( os.path.dirname(sys.argv[1]) )
else:
    print("SYNOPSIS")
    print("\tpermset.py file.json")
    sys.exit()


cfg = read_cfg("test.json")

if ("clean" in cfg) and cfg["clean"]:
    print("CLEAN: all permission to root")
    executeCmd("setfacl -b -R -P ./")
    executeCmd("chmod 755 -R ./")
    executeCmd("chown root:root -R ./")
if "setacl" in cfg:
    setacl(cfg["setacl"])
