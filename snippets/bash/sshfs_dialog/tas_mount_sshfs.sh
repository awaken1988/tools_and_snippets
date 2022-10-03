#!/bin/bash



#sshfs "$TARGET" "$MOUNT_POINT" -o idmap=user,ServerAliveInterval=15 -o uid=$(id -u) -o gid=$(id -g) 

declare -a HOSTS         
HOSTS+=("root@myhost.local:/")       
HOSTS+=("user@otherhost.local:/media/anything")  
HOSTS+=("user1@10.0.0.1:/media/other")     

MENU=""
for ihost in ${HOSTS[*]}; do
    MENU="${MENU}$ihost"$'\n'$'\n'
done

TMPFILE=$(mktemp)
printf "%s" "$MENU" | xargs -d "\n" dialog --menu "Choose" 0 0 0 2> $TMPFILE

MOUNT_NAME=$(cat $TMPFILE | awk -F ":/" '{print $1}')
MOUNT_POINT=$(bash tas_mount_basedir.sh "$MOUNT_NAME")


TARGET=$(cat $TMPFILE)

sshfs "$TARGET" "$MOUNT_POINT" -o idmap=user,ServerAliveInterval=15 -o uid=$(id -u) -o gid=$(id -g) 