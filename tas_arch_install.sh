#!/bin/bash
set -e

#change the settings here
EFI_PARTITION=/dev/loop0p1
ROOT_PARTITION=/dev/loop0p2

#do net change the following of the file
INSTALL_NAME=install_arch
INSTALL_ROOT=/mnt/$INSTALL_NAME/

#prepare disk
cryptsetup luksFormat --type luks1 $ROOT_PARTITION
cryptsetup open $ROOT_PARTITION $INSTALL_NAME
mkfs.btrfs -f /dev/mapper/$INSTALL_NAME

mkdir -p $INSTALL_ROOT
mount /dev/mapper/$INSTALL_NAME $INSTALL_ROOT
pacstrap $INSTALL_ROOT base linux linux-firmware vim btrfs-progs grub 

mkdir -p $INSTALL_ROOT/efi
mount $EFI_PARTITION $INSTALL_ROOT/efi

genfstab -U $INSTALL_ROOT >> $INSTALL_ROOT/etc/fstab
arch-chroot $INSTALL_ROOT   ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime 
#arch-chroot $INSTALL_ROOT   hwclock --systohc
arch-chroot $INSTALL_ROOT   echo "LANG=en_US.UTF-8" > /etc/locale.conf
arch-chroot $INSTALL_ROOT   echo "KEYMAP=de-latin1" > /etc/vconsole.conf
arch-chroot $INSTALL_ROOT   echo "myhost" > /etc/hostname

#should be run at the end
arch-chroot $INSTALL_ROOT   mkinitcpio -P



