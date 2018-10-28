import subprocess
import re
import json
import socket
from mod_base import *
import helper

class ModRouting(ModBase):
    def __init__(self): 
        ModBase.__init__(self)
        
    def get_name(self):
       return "Host Routing-Table "

    def action(self, aActionArg):
        output = (subprocess.run("ip -j -p route", shell=True, capture_output=True).stdout).decode('utf-8')
        parsed = json.loads(output)
        return helper.json_to_html(parsed, ("dst", "dev", "gateway", "protocol", "metric"))
  
