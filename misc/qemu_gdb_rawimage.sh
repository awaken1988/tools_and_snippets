#!/bin/bash

#Start qemu arm with a raw binary

BINARY=$1
ADDRESS=$2
START=$3

qemu-system-arm -M vexpress-a15 -S -s &  
arm-none-eabi-gdb -ex "target remote localhost:1234" -ex "layout reg" -ex "restore $BINARY binary $ADDRESS" -ex "set \$pc=$START"
