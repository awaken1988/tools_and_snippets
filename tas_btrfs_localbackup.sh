#!/bin/bash


SRC_BASE=$1
DEST_BASE=$2
echo "[INFO] SRC_BASE = $SRC_BASE"
echo "[INFO] DEST_BASE = $DEST_BASE"

SRC_BASENAME=$(basename "$SRC_BASE")
SRC_BASE=$SRC_BASE/../.snapshot_$SRC_BASENAME
DEST_BASE=$DEST_BASE/$SRC_BASENAME

SNAPS_CMD="ls -1d $SRC_BASE/*/"
SNAPS=$($SNAPS_CMD | sort)
PREV_SNAP=${SNAPS[0]}

for iSnap in ${SNAPS[@]}; do
    echo "[INFO] -> backup snapshot(not done yet) = $iSnap"
done

echo $DEST_BASE
if [ ! -d "$DEST_BASE/" ]; then
    echo "[INFO] backup full $DEST_BASE"
    echo "[INFO]    full_backup_snap=$PREV_SNAP" 
    
    mkdir -p $DEST_BASE 
    btrfs send -q $PREV_SNAP | btrfs receive $DEST_BASE 1>/dev/null   

    if [ $? -ne 0 ]; then exit 1; fi;
fi


for iSnap in ${SNAPS[@]}; do
    if [ "$iSnap" == "$PREV_SNAP" ]; then continue; fi;
    
    #check if dir or file exists
    SNAP_BASE=$(basename $iSnap)
    if [ -d $DEST_BASE/$SNAP_BASE ]; then
        echo "[INFO] skip; already existing = $DEST_BASE/$SNAP_BASE"
        PREV_SNAP=$iSnap
        continue
    fi 

    echo "[INFO] incremental backup to $BACKUP_SNAP"
    echo "[INFO]    left_src  = $PREV_SNAP"
    echo "[INFO]    right_src = $iSnap"

    btrfs send -qp $PREV_SNAP $iSnap | btrfs receive $DEST_BASE 1>/dev/null   

    if [ $? -ne 0 ]; then exit 1; fi;
    PREV_SNAP=$iSnap
done
