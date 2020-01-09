#!/bin/bash

#
PACKAGES=()
function append {
    PACKAGES+=($1)
}

echo "*** install packages ***"
read -p "Install  BASE_CONSOLE y/n: " IS_BASE_CONSOLE
read -p "Install  BASE_GUI y/n: " IS_BASE_GUI
read -p "Install  DEVELOPER y/n: " IS_DEVELOPER

if [ $IS_BASE_CONSOLE == "y" ]; then
    append    grub
    append    grub-btrfs
    append    wireshark-cli
    append    python
    append    git
    append    screen
    append    nmap
    append    htop
    append    rsync
    append    iotop
    append    cifs-utils
    append    wget
    append    curl
    append    vim
    append    vnstat
    append    iptraf-ng
    append    smbclient
    append    dhcpcd
    append    bind-tools
    append    bridge-utils
    append    cifs-utils
    append    iproute2
    append    iperf3
    append    bluedevil
    append    networkmanager
    append    e2fsprogs
    append    smartmontools
fi

if [ $IS_BASE_GUI == "y" ]; then
    append    xorg-server
    append    xorg-apps
    append    plasma
    append    kde-applications
	append    sddm
    append    firefox
    append    code
    append    vlc
    append    gimp
    append    inkscape
    append    wireshark-qt
    append    nextcloud-client
fi

if [ $IS_DEVELOPER == "y" ]; then
    append    base-devel
	append    cmake
fi

#*** other packates ***
#   conky		                Test
#   remmina [remote]		    Test
#   tigervnc [remote]		    Test
#   krfb [remote]		        Test
#   yaourt [Arch Packages]		Test
#   avahi		                Test
#   bluthooth stuff???		
#		
#   smb4k		NotWork

pacman -Sy
pacman -S --noconfirm --needed ${PACKAGES[*]}

echo "*** checkoout tools_and_snippets"
git clone https://github.com/awaken1988/tools_and_snippets /opt/tas
echo 'export PATH=${PATH}:/opt/tas' >> /etc/profile

echo "*** write /etc/vimrc ***"
cat <<- EOF > /etc/vimrc
	set nu
	set laststatus=2
	set wildmenu
	syntax enable 
EOF

echo "*** write /etc/screenrc ***"
cat <<- EOF > /etc/screenrc
	caption always "%{= kw}%-w%{= gW}%n %t%{-}%+w %-= bigdatums.net - %Y-%m-%d %C:%s"
EOF

if [ $IS_BASE_GUI == "y" ]; then
	systemctl enable sddm
	systemctl enable NetworkManager
fi

echo "*** TODO: install yay ***"
#pushd /tmp
#git clone https://aur.archlinux.org/yay.git
#cd yay
#makepkg -si
#popd

#TODO: install ftp server
#TODO: install app armor???
#TODO: install malware scanner
