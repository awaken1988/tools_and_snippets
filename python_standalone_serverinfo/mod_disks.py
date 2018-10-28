import subprocess
import re
import json
import socket
from mod_base import *
import helper

class ModDisks(ModBase):
    def __init__(self): 
        ModBase.__init__(self)
        
    def get_name(self):
       return "Host Routing-Table "

    def action(self, aActionArg):
        output = (subprocess.run("lsblk -l -J", shell=True, capture_output=True).stdout).decode('utf-8')
        parsed = json.loads(output)["blockdevices"]
        return helper.json_to_html(parsed, ("name", "mountpoint", "size"))
   
