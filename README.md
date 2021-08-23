A collection of several helper scripts, code snippets ... .  

# Install scripts

## tas_arch_install.sh
This script can be run from a archlinux install live cd. It creates a fully encrypted linux installation on UEFI Systems. All the files in /boot are also encrypted
The script will ask for the corresponding block devices. If the script finished without errors you have a fully encrypted system

Prerequisites:
* UEFI GPT Partition Table
* Already formated EFI partition 
* An empty partition for the encrypted btrfs root filesystem

Notes:
* btrfs subvolumes
    * root: / 
    * home: /home
    * pacman_cache: /var/cache/pacman/pkg
* you only have to enter the grub password

TODO:
* EFI bootentry???

## tas_arch_configure.sh
* Install a list of packages with pacman
* write some config: e.g /etc/screenrc

## tas_win10_configure.sh
Install a list of packages with chocolatey on Windows 10 (Runs in Powershell)

Prerequisite: 
* Chocolatey installed. 
* Skript allowed in Powershell

# Other Scripts

## tas_mount.py
A small tkinter GUI for:
* mount windows shares
* connect to ssh

Your write all your mounts in a corresponding .json file.
This script can be used for WINDOWS & LINUX

Todo: Linux not tested

## tas_wireguard_gen.py
create a wireguard configuration with star topology
```
tas_wireguard_gen.py hosts.txt
```

Example of hosts.txt
```
_ifname=private
_server_endpoint=your.server:51820
_ip4=10.222.208.
_ip6=fd00:cafe:0:8::

server      1      1
client0     10     a
client1     20     14
client2     21     15
```
The lines start with _ are global configurations.
* _ifname: Interface name of your tunnel
* _server_endpoint: where your wg server is running
* _ip4: the address without the last part
* _ip6: the address without the last part

For the first entry (here server) a server cfg is configured.
For the following entries client configs are generated.
This script do not generate new keys for clients already created.
So adding clients and rerun the script is possible

Note: ipv6 client to client not yet work with this config

## tas_easyrsa.py
Generates a OpenVPN configuration. (server and clients)

create a ca and server cert
```
tas_easyrsa.py init 
```

create ovpn client configs
```
tas_easyrsa.py add --name mars --config <writen to .ovpn> --config <writen to .ovpn>
```

## tas_http_trigger.py
Standalone http python server which run script on the local host if the secret is correct

## tas_btrfs_*
These tools help you to create snapshots and sync them with another btrfs partition

### Example: tas_btrfs_snap.sh
```
tas_btrfs_snap.sh /mnt/movies
```
This creates a  readonly snapshot under 
> /mnt/.snapshot_movies/2018_12_01__2000

### Example: tas_btrfs_backup.sh
Assume we have created a lot of snapshots as described in the previous example. Now we want to sync them with another btrfs partition over ssh.  
```
tas_btrfs_backup.sh /mnt/movies root@backup.mylan:/media/backup
```
The tool takes all the snapshots from the .snapshot_movies folder and send it to the backup server.

Note: if the backup dir doesn't exist we do a full backup *(btrfs send | ...)* with the first snapshot. Otherwise an incremenal backup is done *(btrfs -p ... ... )*

## tas_bytegenerator.py
Create binary files over a simple tk gui.
Currently there are 2 generators:
* **Random**: Generate a Random file
* **Counter**: Generate a file with a counter. You can specifiy the with of the counter value and the start.

## tas_permset_json.py
this let you define your ACL settings in a .json file. which must be located in the basedir of the folderstructure for which the ACL' is set. 

Every ACL is applied recursive to all subfolder. You find an example of the .json file in the script

# Projects

## cpp_snipptes
Some cpp snippets. Mosly belongs new C++ features.

## python_atlr_test
Test Python with ANTRL to create a toy langueage

*Not actively developed anymore*

## python_standalone_serverinfo
A small standalone http server which lists information about the linux host. 

*Not actively developed anymore*
 
## netbrowser
Scan LAN by looking in the Neighbor-cache of the host or with nmap to find host. Each host is the scanned for Samba-Shares and well known port. Finally the results are listed in a Gui

*Not actively developed anymore*



