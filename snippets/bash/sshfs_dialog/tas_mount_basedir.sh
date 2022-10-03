#!/bin/bash

MDIR=~/tas_mount
RUNDIR=/run/user/$(id -u $USER)/tas_mount

if test ! -d $RUNDIR; then
    mkdir -p "$RUNDIR"
    rm -f $MDIR
fi

if test ! -L $MDIR; then
    ln -s "$RUNDIR" "$MDIR"
fi

MNT_POSTFIX=$(date '+%Y_%m_%d__%H_%M')
if test ! -z $1; then
    MNT_POSTFIX=$1
fi

MOUNT_DIR="$RUNDIR/$MNT_POSTFIX"

mkdir -p "$MOUNT_DIR"

echo "$MOUNT_DIR"