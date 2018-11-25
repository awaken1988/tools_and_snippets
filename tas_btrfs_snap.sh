#!/bin/bash

SUBVOL=$(realpath $1)
BACKUP_BASE=$SUBVOL/../.snapshot_$(basename "$SUBVOL")
BACKUP_SUBVOL=$BACKUP_BASE/$(date "+%Y_%m_%d__%H%M")

#check if SUBVOL is a btrfs subvol path
btrfs subvol show "$SUBVOL" 1> /dev/null 2>/dev/null 
[ $? -ne 0 ] && echo "$SUBVOL is not a btrfs subvolume" && exit 1

#TODO: check if the parentdir lies on the same btrfs filesystem

echo "SUBVOL = $SUBVOL"
echo "BACKUP_BASE = $BACKUP_BASE" 
echo "BACKUP_SUBVOL = $BACKUP_SUBVOL"

mkdir -p "$BACKUP_BASE"

btrfs subvolume snapshot -r "$SUBVOL" "$BACKUP_SUBVOL"