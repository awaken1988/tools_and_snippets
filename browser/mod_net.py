import subprocess
import json 
import socket
import sys
import time
import re

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



if __name__ == "__main__":
    result = get_smb_shares("10.222.201.1")
    print(result)