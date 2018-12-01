# Tools and Snippets
I place all the code here which are to small for an own repository. 

# Scripts

## tas_permset_json.py
this let you define your ACL settings in a .json file. which must be located in the basedir of the folderstructure for which the ACL' set. 

Every ACL is applied recursive to all subfolder. You find an example of the .json file in the script

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

Note: if the backup dir doesn't exist we do a full backup *(btrfs send | ...)* with the first snapshot. Otherwise an incremenal backup done *(btrfs -p ... ... )*

## tas_bytegenerator.py
Create binary files over a simple tk gui.
Currently there are 2 generators:
* **Random**: Generate a Random file
* **Counter**: Generate a file with a counter. You can specifiy the with of the counter value and the start.
