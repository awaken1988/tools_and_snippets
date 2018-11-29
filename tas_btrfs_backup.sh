
SUBVOL=$(realpath $1)
SNAPSHOT_BASE=$SUBVOL/../.snapshot_$(basename "$SUBVOL")
BACKUP_BASE=$(realpath $2)

echo "|INFO| SUBVOL = $SUBVOL"
echo "|INFO| SNAPSHOT_BASE = $SNAPSHOT_BASE" 
echo "|INFO| BACKUP_BASE = $BACKUP_BASE" 



SNAPS=($(ls -d $SNAPSHOT_BASE/*/ | sort ))
PREV_SNAP=${SNAPS[0]}

if [ $(ls -1A "$BACKUP_BASE" | wc -l) -eq 0 ]; then
    echo "|INFO| backup full $BACKUP_BASE"
    echo "|INFO|    full_backup_snap=$PREV_SNAP" 
    btrfs send -q "$PREV_SNAP" | btrfs receive "$BACKUP_BASE"  1> /dev/null
fi

for iSnap in ${SNAPS[@]}; do
    if [ "$iSnap" == "$PREV_SNAP" ]; then continue; fi;
    
    BACKUP_SNAP=$BACKUP_BASE/$(basename $iSnap)
    if [ -f $BACKUP_SNAP ]; then
        echo "|ERROR| expexted a subvolume; found a file in $BACKUP_SNAP"
        exit 1
    fi
    if [ -d $BACKUP_SNAP ]; then
        echo "|INFO| skip; already existing = $iSnap"
        PREV_SNAP=$iSnap
        continue
    fi 

    echo "|INFO| incremental backup to $BACKUP_SNAP"
    echo "|INFO|    left_src  = $PREV_SNAP"
    echo "|INFO|    right_src = $iSnap"

    btrfs send -qp "$PREV_SNAP" "$iSnap" | btrfs receive "$BACKUP_BASE" 1> /dev/null
    PREV_SNAP=$iSnap
done