#!/bin/bash
#TODO: 
#       https://askubuntu.com/questions/799743/how-to-insert-tabs-before-output-lines-from-a-executed-command
#
#References:
#       https://wiki.archlinux.org/index.php/Dm-crypt/System_configuration#Using_encrypt_hook

set -e

#EFI_PARTITION=/dev/sda1
#ROOT_PARTITION=/dev/sda2
#INSTALL_NAME=myarch
#LUKS_PASSWORD=bla

read -p "Enter EFI  Partition device, e.g /dev/loop0p1: " EFI_PARTITION
read -p "Enter ROOT Partition device, e.g /dev/loop0p2: " ROOT_PARTITION
read -p "host/install name: " INSTALL_NAME
read -p "luks password for ROOT_PARTITION (entire root): " -s LUKS_PASSWORD
read -p "root user password: " -s ROOT_PASSWORD

#do net change the following of the file
BTRFS_ROOT=/mnt/${INSTALL_NAME}
INSTALL_ROOT=/mnt/${INSTALL_NAME}_root
BTRFS_PART=/dev/mapper/${INSTALL_NAME}

function exec_archroot {
    echo "#!/bin/bash" >  ${INSTALL_ROOT}/root/_tas_arch_install_tmp_${INSTALL_NAME}.sh
    echo "$1"          >> ${INSTALL_ROOT}/root/_tas_arch_install_tmp_${INSTALL_NAME}.sh
    arch-chroot ${INSTALL_ROOT} bash /root/_tas_arch_install_tmp_${INSTALL_NAME}.sh
    rm -f ${INSTALL_ROOT}/root/_tas_arch_install_tmp_${INSTALL_NAME}.sh
}

#dummp all variables
echo "**** config ****"
echo "    BTRFS_ROOT=$BTRFS_ROOT"
echo "    ROOT_PARTITION=$ROOT_PARTITION"
echo "    INSTALL_NAME=$INSTALL_NAME"
echo "    BTRFS_ROOT=$BTRFS_ROOT"
echo "    INSTALL_ROOT=$INSTALL_ROOT"
echo "    HOME_DIR=$HOME_DIR"
echo "    BTRFS_PART=$BTRFS_PART"

#prepare disk
echo "*** format $ROOT_PARTITION ***"
echo -n $LUKS_PASSWORD | cryptsetup luksFormat --type luks1 ${ROOT_PARTITION} -d -
echo -n $LUKS_PASSWORD | cryptsetup open ${ROOT_PARTITION} ${INSTALL_NAME} -d -
mkfs.btrfs -f /dev/mapper/${INSTALL_NAME}
mkdir -p ${BTRFS_ROOT}
mkdir -p ${INSTALL_ROOT}
mount ${BTRFS_PART} ${BTRFS_ROOT}
btrfs subvolume create ${BTRFS_ROOT}/root
btrfs subvolume create ${BTRFS_ROOT}/home
btrfs subvolume create ${BTRFS_ROOT}/pacman_cache
mount ${BTRFS_PART} ${INSTALL_ROOT}      -o subvol=root

#install base system
echo "*** pacstrap ***"
pacstrap ${INSTALL_ROOT} base linux linux-firmware btrfs-progs grub efibootmgr nano vim cpio dhcpcd
#cp -Ra /mnt/template/* ${INSTALL_ROOT}/

echo "*** mount additional stuff: efi,home ***"
mkdir -p $INSTALL_ROOT/efi
mount $EFI_PARTITION $INSTALL_ROOT/efi
mount ${BTRFS_PART} ${INSTALL_ROOT}/home                     -o subvol=home

mv -fv ${BTRFS_ROOT}/root/var/cache/pacman/pkg/*  ${BTRFS_ROOT}/pacman_cache
mount ${BTRFS_PART} ${INSTALL_ROOT}/var/cache/pacman/pkg     -o subvol=pacman_cache

echo "*** configure system ***"
genfstab -U ${INSTALL_ROOT} >> $INSTALL_ROOT/etc/fstab
exec_archroot   "echo \"LANG=en_US.UTF-8\" > /etc/locale.conf"
exec_archroot   "echo \"KEYMAP=de-latin1\" > /etc/vconsole.conf"
exec_archroot   "echo \"$INSTALL_NAME\" > /etc/hostname"
exec_archroot   "echo \"127.0.0.1 localhost\" > /etc/hosts"
exec_archroot   "echo \"::1		localhost\" > /etc/hosts"
printf "${ROOT_PASSWORD}\n${ROOT_PASSWORD}\n" | passwd -R ${INSTALL_ROOT} root
arch-chroot ${INSTALL_ROOT}   ln -sf /usr/share/zoneinfo/Europe/Berlin /etc/localtime 
arch-chroot ${INSTALL_ROOT}   hwclock --systohc
arch-chroot ${INSTALL_ROOT}   locale-gen

#get UUID
UUID_ROOT=$(blkid ${ROOT_PARTITION} -s UUID -o value)
UUID_BTRFS=$(blkid ${BTRFS_PART} -s UUID -o value)

echo "*** install grub ***"
sed -i 's|#GRUB_ENABLE_CRYPTODISK=y.*|GRUB_ENABLE_CRYPTODISK=y|g' ${INSTALL_ROOT}/etc/default/grub 

dd bs=512 count=4 if=/dev/random of=${INSTALL_ROOT}/root/${INSTALL_NAME}.keyfile iflag=fullblock
chmod 000 ${INSTALL_ROOT}/root/${INSTALL_NAME}.keyfile
echo -n $LUKS_PASSWORD | cryptsetup -v luksAddKey ${ROOT_PARTITION} ${INSTALL_ROOT}/root/${INSTALL_NAME}.keyfile
sed -i "s|FILES=.*|FILES=(/root/${INSTALL_NAME}.keyfile)|g" ${INSTALL_ROOT}/etc/mkinitcpio.conf

GRUB_CMDLINE_LINUX="GRUB_CMDLINE_LINUX=\""
GRUB_CMDLINE_LINUX+="rd.luks.name=/dev/disk/by-uuid/${UUID_ROOT}=${INSTALL_NAME}"
GRUB_CMDLINE_LINUX+="rd.luks.key=/root/${INSTALL_NAME}.keyfile"
GRUB_CMDLINE_LINUX+="\""
sed -i "s|GRUB_CMDLINE_LINUX=.*|$GRUB_CMDLINE_LINUX|g" ${INSTALL_ROOT}/etc/default/grub

GRUB_CMDLINE_LINUX_DEFAULT="GRUB_CMDLINE_LINUX_DEFAULT=\"root=/dev/mapper/${INSTALL_NAME}\""
sed -i "s|GRUB_CMDLINE_LINUX_DEFAULT=.*|$GRUB_CMDLINE_LINUX_DEFAULT|g" ${INSTALL_ROOT}/etc/default/grub

sed -i 's|HOOKS=.*|HOOKS=(base systemd autodetect keyboard sd-vconsole modconf block sd-encrypt filesystems fsck)|g' ${INSTALL_ROOT}/etc/mkinitcpio.conf
echo "${INSTALL_NAME}   /dev/disk/by-uuid/${UUID_ROOT}    /root/${INSTALL_NAME}.keyfile   luks,timeout=30" > ${INSTALL_ROOT}/etc/crypttab.initramfs

arch-chroot $INSTALL_ROOT   grub-install --target=x86_64-efi --efi-directory=/efi --bootloader-id=GRUB --recheck
arch-chroot $INSTALL_ROOT   grub-mkconfig -o /boot/grub/grub.cfg
arch-chroot $INSTALL_ROOT   mkinitcpio -p linux

echo "*** snapshot original installation"
mkdir -p ${BTRFS_ROOT}/snapshots
btrfs subvolume snapshot -r ${BTRFS_ROOT}/root/         ${BTRFS_ROOT}/snapshots/$(date "+%Y_%m_%d__%H_%M__root")
btrfs subvolume snapshot -r ${BTRFS_ROOT}/home/         ${BTRFS_ROOT}/snapshots/$(date "+%Y_%m_%d__%H_%M__home")
btrfs subvolume snapshot -r ${BTRFS_ROOT}/pacman_cache/ ${BTRFS_ROOT}/snapshots/$(date "+%Y_%m_%d__%H_%M__pacman_cache")


