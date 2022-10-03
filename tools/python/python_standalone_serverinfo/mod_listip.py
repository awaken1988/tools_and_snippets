import subprocess
import re
from mod_base import *

def get_interfaces():
    ret = []
    console_output = (subprocess.run("ip link show", shell=True, capture_output=True).stdout).decode('utf-8').splitlines()
    for iLine in console_output:
        macht_if = re.match(r"^\d+: (\w+):.*", iLine)
        if macht_if:
            ret.append(macht_if.group(1))    
    return ret

def get_interface_ips(aInterface):
    ret = []
    console_output = (subprocess.run("ip addr show dev "+aInterface, shell=True, capture_output=True).stdout).decode('utf-8').splitlines()
    for iLine in console_output:
        macht_if = re.match(r" *inet6? ([a-f0-9.:/]+) .*", iLine)
        if macht_if:
            ret.append(macht_if.group(1))
    return ret

def get_interface_ether(aInterface):
    console_output = (subprocess.run("ip addr show dev "+aInterface, shell=True, capture_output=True).stdout).decode('utf-8').splitlines()
    for iLine in console_output:
        macht_if = re.match(r" *link/ether ([a-f0-9:]+) .*", iLine)
        if macht_if:
            return macht_if.group(1)

class ModListip(ModBase):
    def __init__(self): 
        ModBase.__init__(self)

    def get_name(self):
       return "Host IP-Addresses"

    def action(self, aActionArg):
        content = "<table>"
        content += "<tr><th>Interface</th><th>MAC</th></tr>"


        for iIf in get_interfaces():
            content += "<tr>"
            content += "<td>"+iIf+"</td>"

            ether = get_interface_ether(iIf)
            content += "<td>"
            if ether:
                content += ether
            content += "</td>"

            ip_li = ""
            for iIp in get_interface_ips(iIf):
                 ip_li +=  "<li>"+iIp + "</li>"
            content += "<td><ul>"+ip_li+"</ul></td>"
            content += "</tr>"
        content += "</table>"

        return content


  
 