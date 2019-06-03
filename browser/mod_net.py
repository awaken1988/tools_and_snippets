import subprocess
import json 
import socket
import sys
import time
import re
import shutil

def get_neighbors():
    ret = []

    cmd_result = subprocess.run("ip -j neigh", shell=True, capture_output=True)
    cmd_result = json.loads(cmd_result.stdout.decode('utf-8'))
    
    for iEntry in cmd_result:
        if "lladdr" not in iEntry: continue
        if "dev"    not in iEntry: continue
        if "dst"    not in iEntry: continue
        ret.append( {
            "dev": iEntry["dev"],
            "ip": iEntry["dst"],
            "mac": iEntry["lladdr"],
        })        

    return ret

def scan_a_port(aAddress, iPort):
    try:
        s = socket.create_connection((aAddress, iPort), 1)
    except:
        return False
    return True

def get_hostname(aAddr):
    try:
        hostname_query = socket.gethostbyaddr(aAddr)
        return hostname_query[0]
    except:
        pass
    return ""

def get_smb_shares(aAddress):
    ret = []
    cmd_result = subprocess.run("smbtree -N {}".format(aAddress), shell=True, capture_output=True).stdout.decode('utf-8')
    for iLine in cmd_result.split("\n"):
        regex_result = re.search("^[ \t]+\\\\\\\\([a-z0-9_]+)\\\\([a-z0-9_$]+).*", iLine, flags=re.IGNORECASE)
        if not regex_result:
            continue
        ret.append( [regex_result.group(1), regex_result.group(2)] )
    return ret

#def get_first_executable(aExecutableList):
#    for iExec in aExecutableList:
#        if shutil.which(iExec):
#            return iExec
#    return None

class ServiceHandlers:
    @staticmethod
    def ssh(aConnectInfo):
        cmd = "konsole --hold -e ssh "+aConnectInfo["host"] + " -p "+aConnectInfo["port"]
        subprocess.Popen(cmd, shell=True)
    def http(aConnectInfo):
        cmd = "python3 -m webbrowser -n http://"+aConnectInfo["host"]
        subprocess.Popen(cmd, shell=True)
    def https(aConnectInfo):
        cmd = "python3 -m webbrowser -n https://"+aConnectInfo["host"]
        subprocess.Popen(cmd, shell=True)


if __name__ == "__main__":
    ServiceHandlers.https({ "host":"saturn.mkhome", "port": "80"})