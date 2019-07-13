import subprocess
import json

def get_hosts():
    ret = []

    cmd_result = subprocess.run("powershell.exe \"Get-NetNeighbor | Select-Object ifIndex,IPAddress,LinkLayerAddress | ConvertTo-Json\"", shell=True, capture_output=True)
    cmd_result = json.loads(cmd_result.stdout.decode('utf-8'))
    
    for iEntry in cmd_result:
        if "LinkLayerAddress" not in iEntry: continue
        if "ifIndex"    not in iEntry: continue
        if "IPAddress"    not in iEntry: continue

        ipaddr = str(iEntry["IPAddress"])
        macaddr = str(iEntry["LinkLayerAddress"]).replace("-", ":")

        if( ipaddr.startswith("ff")  ):
            continue

        ret.append( {
            "dev": str(iEntry["ifIndex"]),
            "ip": ipaddr,
            "mac": macaddr,
        })        

#    for iHost in ret:
#        #set hostname
#        hostname = get_hostname(iHost["ip"])
#        if hostname:
#            iHost["hostname"] = hostname 
#        iHost["services"] = {}
#
#        #set avail services
#        for iServiceName, iService in SERVICES.items():
#            iHost["services"][iServiceName] = iService["data"].fetchinfo(iHost)
    

    print(ret)
    return ret




get_hosts()