import subprocess
import json

def get_hosts():
    ret = []

    cmd_result = subprocess.run(EXECUTABLES["ip"]["cmd"]+" -j neigh", shell=True, capture_output=True)
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

    #append localhost
    ret.append( {   "dev":      "lo", 
                    "ip":       "127.0.0.1",
                    "mac":   "00:00:00:00:00:00"} )

    for iHost in ret:
        #set hostname
        hostname = get_hostname(iHost["ip"])
        if hostname:
            iHost["hostname"] = hostname 
        iHost["services"] = {}

        #set avail services
        for iServiceName, iService in SERVICES.items():
            iHost["services"][iServiceName] = iService["data"].fetchinfo(iHost)
    

    return ret