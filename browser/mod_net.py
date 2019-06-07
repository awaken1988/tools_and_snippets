import subprocess
import json 
import socket
import sys
import time
import re
import shutil



#def get_smb_shares(aAddress):
#    ret = []
#    cmd_result = subprocess.run("smbtree -N {}".format(aAddress), shell=True, capture_output=True).stdout.decode('utf-8')
#    for iLine in cmd_result.split("\n"):
#        regex_result = re.search("^[ \t]+\\\\\\\\([a-z0-9_]+)\\\\([a-z0-9_$]+).*", iLine, flags=re.IGNORECASE)
#        if not regex_result:
#            continue
#        ret.append( [regex_result.group(1), regex_result.group(2)] )
#    return ret

#def get_first_executable(aExecutableList):
#    for iExec in aExecutableList:
#        if shutil.which(iExec):
#            return iExec
#    return None

#       main = QWidget()
#        lyt = QHBoxLayout(main)
#        for iShare in self.shares:
#            tool = QToolButton()
#            tool.setToolButtonStyle(Qt.ToolButtonTextOnly)
#            tool.setPopupMode(QToolButton.MenuButtonPopup)
#            tool.setText(iShare[1])
#            men = QMenu()
#            curr_action0 = QAction("iShare[1] 0", main)
#            curr_action1 = QAction("iShare[1] 1", main)
#            curr_action2 = QAction("iShare[1] 2", main)
#            men.addAction(curr_action0)
#            men.addAction(curr_action1)
#            men.addAction(curr_action2)
#            tool.setMenu(men)
#            lyt.addWidget(tool)
#        return main


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
    print(get_smb_shares("saturn.mkhome"))