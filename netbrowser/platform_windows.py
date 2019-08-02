import subprocess
import json

class Platform:

    @staticmethod
    def getPlatformExecutables():
        EXECUTABLES = {}
        EXECUTABLES["Get-NetNeighbor"] =    {"cmd": "Get-NetNeighbor",       "required": True}
        EXECUTABLES["nslookup"] =           {"cmd": "nslookup",              "required": False}
        return EXECUTABLES

    @staticmethod
    def which_command(aCommand):
        cmd_result = subprocess.run("powershell.exe \"Get-Command {} | Select-Object Name | ConvertTo-Json\"".format(aCommand), 
            shell=True, capture_output=True)
        cmd_result = json.loads(cmd_result.stdout.decode('utf-8'))

        if "Name" in cmd_result:
            return True
        return False    

    @staticmethod
    def execute_terminal(aCommandLine):
        subprocess.Popen("start powershell "+aCommandLine, shell=True)

    @staticmethod
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

            ret.append( {
                "dev": str(iEntry["ifIndex"]),
                "ip": ipaddr,
                "mac": macaddr,
            })        

            #print(ret[-1])

        return ret

    @staticmethod
    def add_actions(aSERVICES):
        default_actions = []
        executables = Platform.getPlatformExecutables()

        if Platform.which_command( executables["nslookup"]["cmd"] ):
            default_actions.append(  {"service": "domain",      "name": "nslookup",      
                "action":  lambda aInfo: Platform.execute_terminal(executables["nslookup"]["cmd"]+" - "+aInfo["host"]) } )

        for iAction in default_actions:
            if iAction["service"] not in aSERVICES:
                continue
            aSERVICES[iAction["service"]]["data"].add_action(iAction["name"], iAction["action"])

    @staticmethod
    def smbservice_fetchinfo(aHostInfo):
        ret = []

        try:
            cmd_result = subprocess.run("powershell.exe \"get-WmiObject -class Win32_Share -computer {} | Select-Object Name | ConvertTo-Json\"".format(aHostInfo["ip"]), 
                shell=True, capture_output=True)
            cmd_result = json.loads(cmd_result.stdout.decode('utf-8'))

            for iSmb in cmd_result:
                ret.append( iSmb["Name"]  )
        except:
            pass

        return ret
       