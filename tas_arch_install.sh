#!/bin/bash
set -e

read -p "Enter EFI  Partition device, e.g /dev/loop0p1: " EFI_PARTITION
read -p "Enter ROOT Partition device, e.g /dev/loop0p2: " ROOT_PARTITION
read -p "host/install name: " INSTALL_NAME

#do net change the following of the file
INSTALL_ROOT=/mnt/$INSTALL_NAME/

#prepare disk
echo "* luksFormat $ROOT_PARTITION"
cryptsetup luksFormat --type luks1 $ROOT_PARTITION
cryptsetup open $ROOT_PARTITION $INSTALL_NAME
echo "* format /dev/mapper/$INSTALL_NAME with btrfs (luks $ROOT_PARTITION)"
mkfs.btrfs -f /dev/mapper/$INSTALL_NAME

echo "* pacstrap"
mkdir -p $INSTALL_ROOT
mount /dev/mapper/$INSTALL_NAME $INSTALL_ROOT
pacstrap $INSTALL_ROOT base linux linux-firmware vim btrfs-progs grub 

mkdir -p $INSTALL_ROOT/efi
mount $EFI_PARTITION $INSTALL_ROOT/efi

echo "* configure system"
genfstab -U $INSTALL_ROOT >> $INSTALL_ROOT/etc/fstab
arch-chroot $INSTALL_ROOT   ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime 
arch-chroot $INSTALL_ROOT   hwclock --systohc
arch-chroot $INSTALL_ROOT   echo "LANG=en_US.UTF-8" > /etc/locale.conf
arch-chroot $INSTALL_ROOT   echo "KEYMAP=de-latin1" > /etc/vconsole.conf
arch-chroot $INSTALL_ROOT   echo $INSTALL_NAME > /etc/hostname

#should be run at the end
arch-chroot $INSTALL_ROOT   mkinitcpio -P



