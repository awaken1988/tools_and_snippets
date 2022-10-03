#!/usr/bin/python

import os
import sys
import argparse

from os import path


def errorReturn(aMsg):
    print(aMsg)
    sys.exit(1)

def scriptDir():
    return os.path.dirname(__file__)

def scriptRelative(aPath):
    ret = ""
    if os.path.isabs(aPath):    ret = aPath
    else:                       ret = path.abspath(path.join(scriptDir(), aPath))

    if ret.endswith("/"):       ret = ret[0:-1]

def getOptions():
    parser = argparse.ArgumentParser()
    parser.add_argument('--src',  required=True,  help='source dir of the btrfs subvolume')
    parser.add_argument('--dest', required=True,  help='destination dir to backup to. Note that in this dir a dir with SRC name is created')
    args = parser.parse_args()

    return {
        "src":  scriptRelative(args.src),
        "dest": scriptRelative(args.dest)
    }

options = getOptions()

print("{}".format(os.path.dirname(__file__)))

src       = options["src"]
src_name  = path.basename(src)
dest_base = options["dest"]
dest      = path.join(dest_base, src_name)
src_snaps = path.join(src, "/../.snapshot_"+src_name)

if not path.isdir(dest):
    print("Destination dir {} does not exist".format(dest))
    print("should it created: y/n")
    choose = input()
    if choose == "y":   os.mkdir(dest)
    else:               errorReturn("Dir {} required for backup".format(dest))

for iDir in [src, dest, src_snaps]:
    if not os.path.isdir(iDir):
        errorReturn("[ERROR] dir {} does not exists".format(iDir))

snapshots=[]
for idx, iSnap in enumerate(os.listdir(src_snaps)):
    is_first_snap = idx==0

    entry = {}
    
    snap_src  = path.join(src_snaps, iSnap)
    snap_dest = path.join(dest,      iSnap)
    action="none"

    is_dest = path.isdir(snap_dest)

    if is_first_snap and not is_dest:   action="full"
    elif not is_dest:                   action="incremental"

    entry["name"]          = iSnap
    entry["src"]           = snap_src
    entry["dest"]          = snap_dest
    entry["is_dest"]       = is_dest
    entry["action"]        = action

    if not is_first_snap:
        entry["parent"]        = snapshots[idx-1]["name"]
        entry["parent_path"]   = snapshots[idx-1]["src"]
    else:
        entry["parent"]        = ""
        entry["parent_path"]   = ""

    snapshots.append(entry) 

print("SRC      = {}".format(src))
print("DEST     = {}".format(dest))
print("BASENAME = {}".format(src_name))
print("SRC_SNAP = {}".format(src_snaps))
print("SNAPSHOTS")
for iSnap in snapshots:
    print("    {}: action={}; parent={}  ".format(iSnap["name"], iSnap["action"], iSnap["parent"]))


#-------------------
#Execute
#-------------------
print("Are you sure    yes/no/dry")
choose=input()

is_run    = choose == "yes"
is_dryrun = choose == "dry"

if not is_run and  not is_dryrun:
    sys.exit(0)

for idx, iSnap in enumerate(snapshots):
    action  = iSnap["action"]
    is_full = action == "full"
    is_incr = action == "incremental"
  
    command=""

    if is_full:     command="btrfs send -q {}    | btrfs receive {}".format(iSnap["src"], dest)
    elif is_incr:   command="btrfs send -p {} {} | btrfs receive {}".format(iSnap["parent_path"], iSnap["src"], dest)
    else:           continue
    
    print(command)

    if is_dryrun:   continue
    elif is_run:    os.system(command)
