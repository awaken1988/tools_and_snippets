import subprocess
import re
import json
import socket
from mod_base import *

def list_lan_ips(aAddressRange):
    ret = []
    console_output = (subprocess.run("nmap -sn -n "+aAddressRange, shell=True, capture_output=True).stdout).decode('utf-8').splitlines()
    print(console_output)
    for iLine in console_output:
        print(iLine)
        macht_if = re.match(r"Nmap scan report for ([0-9a-z.:]+)", iLine)
        if macht_if:
            ret.append(macht_if.group(1))    
    return ret

def find_mac(aIpAddr):
    output = (subprocess.run("ip -j -p neigh", shell=True, capture_output=True).stdout).decode('utf-8')
    parsed = json.loads(output)
    
    for iEntry in parsed:
        if iEntry["dst"] == aIpAddr:
            return iEntry

def find_hostname(aIpAddr):
    try:
        return socket.gethostbyaddr(aIpAddr)[0]
    except:
        pass

class ModExploreLan(ModBase):
    def __init__(self, aAddressRange): 
        ModBase.__init__(self)
        self.addressrange = aAddressRange

    def get_name(self):
       return "nmap portscan "+self.addressrange

    def action(self, aActionArg):
        output = "<table>"
        output += "<tr><th>IP</th><th>Hostname</th><th>MAC</th><th>dev</th></tr>"


        for iIp in list_lan_ips(self.addressrange):
            output += "<tr>"
            output += "<td>"+iIp+"</td>"

            hostname = find_hostname(iIp);
            if hostname:
                output += "<td>"+hostname+"</td>"
            else:
                output += "<td></td>"


            neigh = find_mac(iIp)
            if neigh and "lladdr" in neigh and "dev" in neigh:
                output += "<td>"+neigh["lladdr"]+"</td>"
                output += "<td>"+neigh["dev"]+"</td>"
            else:
                output += "<td></td><td></td>"

            output += "</tr>"

        output += "</table>"
        
        return output




  
find_mac("10.0.2.15")