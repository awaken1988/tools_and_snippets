import subprocess
import json

def getPlatformExecutables():
    EXECUTABLES = {}
    EXECUTABLES["Get-NetNeighbor"] =    {"cmd": "Get-NetNeighbor",       "required": True}
    return EXECUTABLES

def which_command(aCommand):
    cmd_result = subprocess.run("powershell.exe \"Get-Command {} | Select-Object Name | ConvertTo-Json\"".format(aCommand), 
        shell=True, capture_output=True)
    cmd_result = json.loads(cmd_result.stdout.decode('utf-8'))

    if "Name" in cmd_result:
        return True
    return False    

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

        #TODO: move this to tas_network_browser.py
        if ipaddr.startswith("ff"):         continue
        if ipaddr == "255.255.255.255":     continue
        for iMult4 in range(224, 239+1):
            if ipaddr.startswith("{}.".format(iMult4)):
                continue

        ret.append( {
            "dev": str(iEntry["ifIndex"]),
            "ip": ipaddr,
            "mac": macaddr,
        })        

    return ret




get_hosts()