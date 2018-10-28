import subprocess
import re
from mod_base import *

def list_lan_ips(aAddressRange):
    ret = []
    console_output = (subprocess.run("nmap -sn -n "+aAddressRange, shell=True, capture_output=True).stdout).decode('utf-8').splitlines()
    for iLine in console_output:
        print(iLine)
        macht_if = re.match(r"Nmap scan report for ([0-9a-z.:]+)", iLine)
        if macht_if:
            ret.append(macht_if.group(1))    
    return ret

class ModExploreLan(ModBase):
    def __init__(self, aAddressRange): 
        ModBase.__init__(self)
        self.addressrange = aAddressRange

    def get_name(self):
       return "nmap portscan "+self.addressrange

    def action(self, aActionArg):
        output = "<table>"


        for iIp in list_lan_ips(self.addressrange):
            output += "<tr>"
            output += "<td>"+iIp+"</td>"
            output += "</tr>"

        output += "</table>"
        
        return output




  
 