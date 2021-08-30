#TODO: copy arch package names from 1_archlinux_install_software.sh

import os
import sys
import re

dist = (
    "ID=ubuntu",
    "ID=arch"
)

pkg = {
    "desktop": (
        ("firefox",                                  ""),
        ("p7zip p7zip-full p7zip-rar",               ""),
        ("vlc",                                      ""),
        ("filezilla",                                ""),
        ("kate",                                     ""),
        ("gimp",                                     ""),
        ("inkscape",                                 ""),
        ("wireshark-qt",                             ""),
        ("okteta",                                   ""),
        ("cool-retro-term",                          ""),
        ("kde-config-systemd",                       ""), 
        (   "!snap install --classic code",               
            "code"),
    ),
    "terminal": (
        ("git",                "", ),      
        ("gawk",               "", ),       
        ("tmux",               "", ),       
        ("neovim",             "", ),         
        ("htop",               "", ),       
        ("iotop",              "", ),        
        ("rsync",              "", ),        
        ("nmap",               "", ),       
        ("iproute2",           "", ),           
        ("iperf3",             "", ),         
        ("iptraf-ng",          "", ),            
        ("dialog",             "", ),         
        ("cpufrequtils",       "", ),               
        ("sudo",               "", ),       
        ("hwinfo",             "", ),         
    ),
    "dev": (
        ("build-essential",    "", ),             
        ("cmake",              "", ),     
        ("cargo",              "", ),     
        ("rustc",              "", ),     
        ("rust-gdb",           "", ),         
    ),
    "misc": (
        ("smartmontools",      "", ),             
        ("vnstat",             "", ),     
    ),
}

if len(sys.argv) < 2:
    print("")
    print("1_install_software.py [ubuntu|arch] pkg_category1 pkg_category2 pkg_category_n")
    print("")
    for i_cat in pkg:
       print("Category {}".format(i_cat))
       for i_pkg in pkg[i_cat]:
           print("\t{}".format(i_pkg))
else:
    didx=-1
    cmd=""
    if sys.argv[1] == "ubuntu":
        didx=0
        cmd="apt install"
    elif sys.argv[1] == "arch":
        didx=1
        cmd="pacman -Syu"
    else:
        raise "only ubuntu or arch possible"

    install=""
    special=[]

    used=sys.argv[2:]
    if used[0] == "all":
        used=pkg.keys()

    for i_cat in used:
        for i_pkg in pkg[i_cat]:
            curr_pkg=i_pkg[didx]

            if curr_pkg.startswith("!"):
                curr_pkg=curr_pkg[1:]
                special.append(curr_pkg)
            else: 
                install += curr_pkg + " "

    cmd += " " + install 
    os.system(cmd)
    for i in special:
        os.system(i)






    

