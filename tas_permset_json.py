#!/usr/bin/python

#example of a json configuration
#   the script should located in the parent folder
#
#
# {
#     "setacl" :  
#     [
#         {
#             "acl":          [ "u:martin:rwx" ],
#             "default_acl":  [ "u:martin:rwx" ],
#             "path":         [
#                                 "free", "non-free", "private", "Dropbox", "purchased", "vm"
#                             ]    
#         },
#         {
#             "acl" :           [ "u:martin:rwx", "u:pcguest:r-x", "u:http:r-x" ],
#             "default_acl" :   [ "u:martin:rwx", "u:pcguest:r-x", "u:http:r-x" ],
#             "path":     [   "non-free/music", 
#                             "non-free/games",
#                             "non-free/software",
#                             "non-free/videos/movies",
#                             "non-free/videos/dokus",
#                             "non-free/videos/serien",
#                             "non-free/videos/music",
#                             "non-free/ebookz",
#                             "vm"  ]
#         }
#     ],
# 
#     "clean" : true
# 
# }







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
    print(aCommand)
    #return
    vRet = subprocess.call(aCommand, shell=True)
    if vRet != 0:
        raise Exception(aCommand)

def setacl(aDir, aAcl, aIsRecursive=False, aIsDefault=False):
    cmd = "setfacl "

    if aIsRecursive:    cmd+="-R "
    else:               cmd+="   "

    if aIsDefault:      cmd+="-d "
    else:               cmd+="   "

    cmd+= "-m " + aAcl + " ";
    cmd+= aDir + " ";

    executeCmd(cmd)

def executeJson(aSubCfg):
    for iEntry in aSubCfg:
        #create dir list
        dirs = []
        for iPath in iEntry["path"]:
            dirs = dirs + [(iPath, True)]
            for iSubDir in sub_path_list(iPath):
                dirs = dirs + [(iSubDir, False)]

        #create acl
        if "acl" in iEntry:
            for iAcl in iEntry["acl"]:
                for iDir in dirs:
                    setacl(iDir[0], iAcl, iDir[1], False)

        #create default acl
        if "default_acl" in iEntry:
            for iAcl in iEntry["default_acl"]:
                for iDir in dirs:
                    setacl(iDir[0], iAcl, iDir[1], True)

if len(sys.argv) > 1 and sys.argv[1].endswith("json"):
    os.chdir( os.path.dirname(sys.argv[1]) )
else:
    print("SYNOPSIS")
    print("\tpermset.py file.json")
    sys.exit()


cfg = read_cfg(os.path.basename(sys.argv[1]))

if ("clean" in cfg) and cfg["clean"]:
    print("CLEAN: all permission to root")
    executeCmd("setfacl -b -R -P ./*")
    executeCmd("chmod 750 -R ./*")
    executeCmd("chown root:root -R ./*")
if "setacl" in cfg:
    executeJson(cfg["setacl"])
