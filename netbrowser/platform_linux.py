import subprocess
import json
import shutil
import re

class Platform:

    @staticmethod
    def getPlatformExecutables():
        EXECUTABLES = {}
        EXECUTABLES["ip"] =         {"cmd": "ip",       "required": True}
        EXECUTABLES["dolphin"] =    {"cmd": "dolphin",  "required": False}
        EXECUTABLES["smbtree"] =    {"cmd": "smbtree", "required": False}
        EXECUTABLES["nslookup"] =   {"cmd": "nslookup", "required": False}
        EXECUTABLES["dig"] =        {"cmd": "nslookup", "required": False}
        EXECUTABLES["ping"] =        {"cmd": "nslookup", "required": False}


        return EXECUTABLES

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
    def add_platform_actions(aSERVICES):
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