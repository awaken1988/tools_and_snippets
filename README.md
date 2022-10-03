A collection of several helper scripts, code snippets ... .  

# Helper Scripts

## arch_install.sh
This script can be run from a archlinux install live cd. It creates a fully encrypted linux installation on UEFI Systems. All the files in /boot are also encrypted.
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

## http_trigger.py
Standalone http python server which run script on the local host if the secret is correct

## btrfs_*
These tools help you to create snapshots and sync them with another btrfs partition

### Example: btrfs_snap.sh
```
btrfs_snap.py /mnt/movies /mnt/documents
```
This creates a  readonly snapshot under 
> /mnt/.snapshot_movies/2018_12_01__2000
> /mnt/.snapshot_documents/2018_12_01__2000

### Example: btrfs_backup.py
Assume we have created a lot of snapshots as described in the previous example. With this tool we sync the snapshots with a destination.
That means do incremental backups between btrfs hardrives
```
btrfs_backup.py --src /mnt/movies --dest /media/backup
```
The tool takes all the snapshots from the .snapshot_movies folder and send it to the backup server.

Note: if the backup dir doesn't exist we do a full backup *(btrfs send | ...)* with the first snapshot. Otherwise an incremenal backup is done *(btrfs -p ... ... )*

## bytegenerator.py
Create binary files over a simple tk gui.
Currently there are 2 generators:
* **Random**: Generate a Random file
* **Counter**: Generate a file with a counter. You can specifiy the with of the counter value and the start.

## permset_json.py
this let you define your ACL settings in a .json file. which must be located in the basedir of the folderstructure for which the ACL' is set. 

Every ACL is applied recursive to all subfolder. You find an example of the .json file in the script

# Snippets
Codesnippets in various programming languages.

# Tools
Small tools. 
Note: no active development



