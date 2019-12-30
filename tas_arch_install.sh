#!/bin/bash
set -e

EFI_PARTITION=/dev/sda1
ROOT_PARTITION=/dev/sda2
INSTALL_NAME=myarch
LUKS_PASSWORD=bla

#read -p "Enter EFI  Partition device, e.g /dev/loop0p1: " EFI_PARTITION
#read -p "Enter ROOT Partition device, e.g /dev/loop0p2: " ROOT_PARTITION
#read -p "host/install name: " INSTALL_NAME
#read -p "luks password for ROOT_PARTITION (entire root): " -s LUKS_PASSWORD
#TODO: read -p "root user password: " -s LUKS_PASSWORD

#do net change the following of the file
BTRFS_ROOT=/mnt/${INSTALL_NAME}
INSTALL_ROOT=/mnt/${INSTALL_NAME}_root
BTRFS_PART=/dev/mapper/${INSTALL_NAME}

#dummp all variables
echo "BTRFS_ROOT=$BTRFS_ROOT"
echo "ROOT_PARTITION=$ROOT_PARTITION"
echo "INSTALL_NAME=$INSTALL_NAME"
echo "BTRFS_ROOT=$BTRFS_ROOT"
echo "INSTALL_ROOT=$INSTALL_ROOT"
echo "HOME_DIR=$HOME_DIR"
echo "BTRFS_PART=$BTRFS_PART"

#prepare disk
echo $LUKS_PASSWORD | cryptsetup luksFormat --type luks1 ${ROOT_PARTITION} -d -
echo $LUKS_PASSWORD | cryptsetup open ${ROOT_PARTITION} ${INSTALL_NAME} -d -
mkfs.btrfs -f /dev/mapper/${INSTALL_NAME}
mkdir -p ${BTRFS_ROOT}
mkdir -p ${INSTALL_ROOT}
mount ${BTRFS_PART} ${BTRFS_ROOT}
btrfs subvolume create ${BTRFS_ROOT}/root
btrfs subvolume create ${BTRFS_ROOT}/home
mount ${BTRFS_PART} ${INSTALL_ROOT}      -o subvol=root

#install base system
echo "* pacstrap"
pacstrap ${INSTALL_ROOT} base linux linux-firmware btrfs-progs grub nano

#mount additional stuff: efi,home
mkdir -p $INSTALL_ROOT/efi
mount $EFI_PARTITION $INSTALL_ROOT/efi
mount ${BTRFS_PART} $INSTALL_ROOT/home -o subvol=home

#configure system
genfstab -U $INSTALL_ROOT >> $INSTALL_ROOT/etc/fstab
arch-chroot $INSTALL_ROOT   ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime 
arch-chroot $INSTALL_ROOT   hwclock --systohc
arch-chroot $INSTALL_ROOT   echo "LANG=en_US.UTF-8" > /etc/locale.conf
arch-chroot $INSTALL_ROOT   locale-gen
arch-chroot $INSTALL_ROOT   echo "KEYMAP=de-latin1" > /etc/vconsole.conf
arch-chroot $INSTALL_ROOT   echo $INSTALL_NAME > /etc/hostname
arch-chroot $INSTALL_ROOT   echo "127.0.0.1 localhost" > /etc/hosts
arch-chroot $INSTALL_ROOT   echo "::1		localhost" > /etc/hosts

#echo "root user password"
#arch-chroot $INSTALL_ROOT   passwd

#should be run at the end
arch-chroot $INSTALL_ROOT   mkinitcpio -P



