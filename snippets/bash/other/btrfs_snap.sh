#!/bin/bash

SUBVOL=$(realpath $1)
SNAPSHOT_BASE=$SUBVOL/../.snapshot_$(basename "$SUBVOL")
SNAPSHOT_SUBVOL=$SNAPSHOT_BASE/$(date "+%Y_%m_%d__%H%M")

#check if SUBVOL is a btrfs subvol path
btrfs subvol show "$SUBVOL" 1> /dev/null 2>/dev/null 
[ $? -ne 0 ] && echo "$SUBVOL is not a btrfs subvolume" && exit 1

#TODO: check if the parentdir lies on the same btrfs filesystem

echo "[INFO] SUBVOL = $SUBVOL"
echo "[INFO] SNAPSHOT_BASE = $SNAPSHOT_BASE" 
echo "[INFO] SNAPSHOT_SUBVOL = $SNAPSHOT_SUBVOL"

mkdir -p "$SNAPSHOT_BASE"

btrfs subvolume snapshot -r "$SUBVOL" "$SNAPSHOT_SUBVOL"