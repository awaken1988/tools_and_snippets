import os
import sys
from datetime import datetime
from os import path

now = datetime.now().strftime("%Y_%m_%d__%H%M")

actions = []

for iDir in sys.argv[1:]:
    subvol    = iDir
    if not path.isabs(subvol):  subvol = path.join(os.getcwd(), subvol)
    if subvol.endswith("/"):    subvol = subvol[0:-1]

    name      = path.basename(subvol)
    snapbase  = path.join(".snapshots/{}".format(name))
    snap      = path.join(snapbase, now)
    
    print(snapbase)
 
    if not path.isdir(snapbase):
        os.mkdir(snapbase)

    entry = {}
    entry["cmd"]      = "btrfs subvolume snapshot -r {} {}".format(subvol, snap)
    entry["snapbase"] = snapbase 
    actions.append(entry)

print("Execute Actions? (y/n):")
for iAction in actions:
    print("    {}".format(iAction["cmd"]))
choose=input()

if choose == "y":
    for iAction in actions:
        if not path.isdir(iAction["snapbase"]): os.mkdir(iAction["snapbase"])
        os.system(iAction["cmd"])

