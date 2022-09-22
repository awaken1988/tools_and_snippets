#!/usr/bin/python

import os
import sys

src  = sys.argv[1]
dest = sys.argv[2]

if src[-1]  == "/": src  = src[:-1]
if dest[-1] == "/": dest = dest[:-1]

src_name  = os.path.basename(src)
src_snaps = src + "/../.snapshot_"+src_name

for iDir in [src, dest, src_snaps]:
    if not os.path.isdir(iDir): 
        print("[ERROR] dir {} does not exists".format(iDir)); 
        sys.exit(1)

snapshots=[]
for idx, iSnap in enumerate(os.listdir(src_snaps)):
    is_first_snap = idx==0

    entry = {}
    
    snap_src  = src_snaps  + "/" + iSnap
    snap_dest = dest       + "/" + iSnap
    action="none"

    is_dest = os.path.isdir(snap_dest)

    if is_first_snap and not is_dest:
        action="full"
    elif not is_dest:
        action="incremental"


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

    if is_full:
        command="btrfs send -q {} | btrfs receive {}".format(iSnap["src"], dest)
    elif is_incr:
        command="btrfs send -p {} {} | btrfs receive {}".format(iSnap["parent_path"], iSnap["src"], dest)
    else: continue
    
    print(command)

    if is_dryrun:
        continue
    elif is_run:
        os.system(command)
