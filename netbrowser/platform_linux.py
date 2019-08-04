import subprocess
import json
import shutil
import re
from helper import *

class Platform:

    #TODO: rename
    @staticmethod
    def getPlatformExecutables():
        executables = {}
        executables["ip"] =         {"cmd": "ip",       "required": True}
        executables["dolphin"] =    {"cmd": "dolphin",  "required": False}
        executables["smbtree"] =    {"cmd": "smbtree",  "required": False}
        executables["nslookup"] =   {"cmd": "nslookup", "required": False}
        executables["dig"] =        {"cmd": "nslookup", "required": False}
        executables["ping"] =       {"cmd": "ping",     "required": False}
        executables["mount"] =      {"cmd": "mount",    "required": False}
        return executables

    @staticmethod
    def getActions():
        action = {}
        executables = Platform.getPlatformExecutables()
        exeterm = Platform.execute_terminal

        if Platform.which_command(executables["nslookup"]["cmd"]):
            add_list(action, "domain", {
                "name": "nslookup",
                "exec":  lambda aInfo: exeterm(executables["nslookup"]["cmd"] + " - " + aInfo["host"])
            })

        if Platform.which_command(executables["dig"]["cmd"]):
            add_list(action, "domain", {
                "name": "dig",
                "exec":  lambda aInfo: exeterm(executables["dig"]["cmd"] + " " + aInfo["host"])
            })

        if Platform.which_command(executables["ping"]["cmd"]):
            add_list(action, "ip", {
                "name": "ping",
                "exec":  lambda aInfo: exeterm(executables["ping"]["cmd"] + " " + aInfo["ip"])
            })

        if Platform.which_command(executables["mount"]["cmd"]):
            add_list(action, "smb", {
                "name": "mount",
                "exec": lambda a: print(a)
            })

        return action
   
    @staticmethod
    def which_command(aCommand):
        if not shutil.which(aCommand):
            return False
        return True

    @staticmethod
    def get_hosts():
        ret = []

        executables = Platform.getPlatformExecutables()
        cmd_result = subprocess.run(executables["ip"]["cmd"]+" -j neigh", shell=True, capture_output=True)
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
    

    @staticmethod
    def add_actions(aSERVICES):
        default_actions = []
        executables = Platform.getPlatformExecutables()
        exeterm = Platform.execute_terminal

        #nslookup
        if Platform.which_command(executables["nslookup"]["cmd"]):
            default_actions.append({"service": "domain", "name": "nslookup",
                                    "action": lambda aInfo: exeterm(executables["nslookup"]["cmd"] + " - " + aInfo["host"])} ) 

        #dig
        if Platform.which_command(executables["dig"]["cmd"]):
            default_actions.append({"service": "domain", "name": "dig",
                                    "action": lambda aInfo: exeterm(executables["dig"]["cmd"] + " " + aInfo["host"])} )

        #ping
        if Platform.which_command(executables["ping"]["cmd"]):
            default_actions.append({"service": "ip", "name": "ping",
                                    "action": lambda aInfo: exeterm(executables["ping"]["cmd"] + " " + aInfo["ip"])} )

        default_actions.append({
            "service": "smb", 
            "name": "mount", 
            "action": lambda a: print("bla"),
        })


        for iAction in default_actions:
            if iAction["service"] not in aSERVICES:
                continue
            aSERVICES[iAction["service"]]["data"].add_action(iAction["name"], iAction["action"])



    @staticmethod
    def execute_terminal(aCommandLine):
        print(aCommandLine)
        subprocess.Popen("xterm -hold -e "+aCommandLine, shell=True)


    class SmbService:
        @staticmethod
        def mount_action(aHostInfo):
            print("mount")
            pass

        @staticmethod
        def available(aExecutables):
            if "smbtree" in aExecutables:
                return True
            return False

        @staticmethod
        def fetchinfo(aHostInfo):
            ret = []
            cmd_result = subprocess.run("smbtree -N {}".format(aHostInfo["ip"]), shell=True, capture_output=True).stdout.decode('utf-8')
            for iLine in cmd_result.split("\n"):
                regex_result = re.search("^[ \t]+\\\\\\\\([a-z0-9_]+)\\\\([a-z0-9_$]+).*", iLine, flags=re.IGNORECASE)
                if not regex_result:
                    continue
                ret.append( regex_result.group(2) )
            return ret





