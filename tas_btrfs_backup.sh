#!/bin/bash

function ssh_execute() {
    if [ -n "$1" ]; then
        ssh "$1" "$2"
    else
        $2
    fi
}

#TODO: split SRC and DEST in a loop
#SRC PART
SRC_HOST=
SRC_BASE=$1
if [ -n "$(echo $1 | grep ":")" ]; then
    SRC_HOST=$(echo $1 | cut -d":" -f1)
    SRC_BASE=$(echo $1 | cut -d":" -f2)
fi
echo "[INFO] SRC_HOST = $SRC_HOST (if empty, use local filesystem)"
echo "[INFO] SRC_BASE = $SRC_BASE"

#DST PART
DEST_HOST=
DEST_BASE=$1
if [ -n "$(echo $2 | grep ":")" ]; then
    DEST_HOST=$(echo $2 | cut -d":" -f1)
    DEST_BASE=$(echo $2 | cut -d":" -f2)
fi

#adapat paths
SRC_BASENAME=$(basename "$SRC_BASE")
SRC_BASE=$SRC_BASE/../.snapshot_$SRC_BASENAME
DEST_BASE=$DEST_BASE/$SRC_BASENAME
ssh_execute "$DEST_HOST" "mkdir -p $DEST_BASE "

echo "[INFO] DEST_HOST = $DEST_HOST (if empty, use local filesystem)"
echo "[INFO] DEST_BASE = $DEST_BASE"

SNAPS_CMD="ls -1d $SRC_BASE/*/"
SNAPS=($(ssh_execute "$SRC_HOST" "$SNAPS_CMD" | sort))
PREV_SNAP=${SNAPS[0]}

for iSnap in ${SNAPS[@]}; do
    echo "[INFO] -> backup snapshot(not done yet) = $iSnap"
done

CHECK_DEST_EMPTY_CMD="ls -1d $DEST_BASE/*/ 2> /dev/null"
CHECK_DEST_RESULT=$(ssh_execute "$DEST_HOST" "$CHECK_DEST_EMPTY_CMD")
if [ -z "$CHECK_DEST_RESULT" ]; then
    echo "[INFO] backup full $DEST_HOST:$DEST_BASE"
    echo "[INFO]    full_backup_snap=$PREV_SNAP" 
    
    ssh_execute "$SRC_HOST" "btrfs send -q $PREV_SNAP" |\
      ssh_execute "$DEST_HOST" "btrfs receive $DEST_BASE 1>/dev/null"   

    if [ $? -ne 0 ]; then exit 1; fi;
fi


for iSnap in ${SNAPS[@]}; do
    if [ "$iSnap" == "$PREV_SNAP" ]; then continue; fi;
    
    #check if dir or file exists
    SNAP_BASE=$(basename $iSnap)
    ssh_execute "$DEST_HOST" "[ -d $DEST_BASE/$SNAP_BASE ]"
    if [ $? -eq 0 ]; then
        echo "[INFO] skip; already existing = $DEST_HOST:$DEST_BASE/$SNAP_BASE"
        PREV_SNAP=$iSnap
        continue
    fi 

    echo "[INFO] incremental backup to $BACKUP_SNAP"
    echo "[INFO]    left_src  = $PREV_SNAP"
    echo "[INFO]    right_src = $iSnap"

    ssh_execute "$SRC_HOST" "btrfs send -qp $PREV_SNAP $iSnap" |\
        ssh_execute "$DEST_HOST" "btrfs receive $DEST_BASE 1>/dev/null"   

    if [ $? -ne 0 ]; then exit 1; fi;
    PREV_SNAP=$iSnap
done