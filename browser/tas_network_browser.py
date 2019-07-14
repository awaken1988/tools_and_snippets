#References
#   Qt:
#       https://stackoverflow.com/questions/28565254/how-to-use-qt-webengine-and-qwebchannel
#       https://doc.qt.io/qt-5/qtwebchannel-javascript.html
#       https://stackoverflow.com/questions/31928444/qt-qwebenginepagesetwebchannel-transport-object
#       https://code.woboq.org/qt5/qtwebchannel/examples/webchannel/shared/websocketclientwrapper.cpp.html https://doc.qt.io/qt-5/qtwebchannel-standalone-main-cpp.html
import enum
import subprocess
import json 
import socket
import sys
import time
import platform
import re
import shutil
from PySide2.QtWidgets          import (QLineEdit, QPushButton, QApplication, 
                                        QVBoxLayout, QHBoxLayout, QDialog, QTableView, QGridLayout, 
                                        QLabel, QWidget, QAction, QMenu, QToolButton,
                                        QComboBox, QToolBar, QFrame, QSystemTrayIcon, QStyle)
from PySide2.QtCore             import (QObject, QAbstractTableModel, QModelIndex, Qt, Signal, Slot, SIGNAL)
from PySide2.QtWebEngineWidgets import (QWebEngineView, QWebEnginePage, QWebEngineProfile, QWebEngineScript)
from PySide2.QtWebChannel       import (QWebChannel)
from PySide2.QtWebSockets       import (QWebSocketServer)
from PySide2.QtNetwork          import (QHostAddress)

#include platform specific stuff
if platform.system() == "Linux":    
    from platform_linux import *
elif platform.system() == "Windows":
    from platform_windows import *
else:
    raise Exception("Your Platform not yet supported")

SERVICES = {}
EXECUTABLES = getPlatformExecutables()

def add_fetchinfo(aFetchInfo, aExecutableName, aCommand):
    if aExecutableName not in EXECUTABLES:
        return

    ret  = {
        "name": EXECUTABLES[aExecutableName]["cmd"],
        "exec": lambda aServiceInfo:  subprocess.Popen(aCommand(aExecutableName, aServiceInfo), shell=True)
    }
    aFetchInfo["actions"].append( ret )
   

#the last step: throw away commands that not avail
def cleanup_executables(aExecutable):
    for iKey in [x for x in aExecutable]:
        if which_command(aExecutable[iKey]["cmd"]):
            print("+Command={}".format(aExecutable[iKey]["cmd"]))
        else:
            print("-Command={}".format(aExecutable[iKey]["cmd"]))
            if aExecutable[iKey]["required"]:
                raise Exception("Error: Command={} are required but missing on your system".format(aExecutable[iKey]["cmd"]))
            del aExecutable[iKey]

cleanup_executables(EXECUTABLES)

#------------------------------------
# sys helper
#------------------------------------



#------------------------------------
# net helper
#------------------------------------
def get_host_summary():
    ret = []

    ret = get_hosts()

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

#get hostname, ip
def get_any_host_id(aHostInfo):
    if "hostname" in aHostInfo: 
        return aHostInfo["hostname"]
    return aHostInfo["ip"]


def scan_a_port(aAddress, iPort):
    try:
        s = socket.create_connection((aAddress, iPort), 1)
    except:
        return False
    return True

def get_hostname(aAddr):
    return ""
    try:
        hostname_query = socket.gethostbyaddr(aAddr)
        return hostname_query[0]
    except:
        pass
    return ""

class QContextMenuLabel(QLabel):
    def __init__(self, aServiceInfo):
        QLabel.__init__(self)
        self.service_info = aServiceInfo

    def contextMenuEvent(self, event):
        menu = QMenu(self)
        for iActions in self.service_info["actions"]:
            action = QAction(iActions["name"], self)
            a = lambda func,data: lambda x: func(data) 

            action.triggered.connect( a(iActions["exec"], self.service_info ) )

            menu.addAction(action)

            
        
        menu.exec_(event.globalPos())
        print(str(event.globalPos()))

#------------------------------------
# services
#------------------------------------
class PortServie:
    def __init__(self, aPortInfo):
        self.port_info = aPortInfo

    def fetchinfo(self, aHostInfo):
        ret = []

        if not scan_a_port(aHostInfo["ip"], self.port_info["port"]):
           return ret

        ret.append(  {  "host":         aHostInfo["ip"],
                        "display_name": self.port_info["name"],
                        "actions":      self.port_info["actions"],
        })

        return ret

class SmbHostService:
    @staticmethod
    def fetchinfo(aHostInfo):
        if scan_a_port( get_any_host_id(aHostInfo), 445 ):
            ret = {"host":         get_any_host_id(aHostInfo), 
                    "display_name": "smbserver",
                    "actions": []}

            add_fetchinfo(ret, "dolphin", lambda aCmd,aInfo: "{} smb://{}".format(aCmd, aInfo["host"])   )
            return [ret]
        return []
   
class SmbService:
    @staticmethod
    def action_print1(aServiceInfo):
        print("1_mount "+ str(aServiceInfo))
    
    @staticmethod
    def action_print2(aServiceInfo):
        print("2_mount "+ str(aServiceInfo))

    @staticmethod
    def fetchinfo(aHostInfo):
        ret = []
        cmd_result = subprocess.run("smbtree -N {}".format(aHostInfo["ip"]), shell=True, capture_output=True).stdout.decode('utf-8')
        for iLine in cmd_result.split("\n"):
            regex_result = re.search("^[ \t]+\\\\\\\\([a-z0-9_]+)\\\\([a-z0-9_$]+).*", iLine, flags=re.IGNORECASE)
            if not regex_result:
                continue
            ret.append(  {  "host":         aHostInfo["ip"],
                            "display_name": regex_result.group(2),
                            "actions":       [
                                {"name": "print_v1", "exec":  SmbService.action_print1},
                                {"name": "print_v2", "exec":  SmbService.action_print2},
                            ],
                            "smb_path":     [regex_result.group(1), regex_result.group(2)]} )

        return ret

class ServiceHelper:
    @staticmethod
    def display(aServiceDefinition):
        main = QWidget()
        lyt = QGridLayout(main)
        row = 0
        col = 0
        for iService in aServiceDefinition:
            lbl = QContextMenuLabel(iService)
            lbl.setText(iService["display_name"])
            lbl.setFrameStyle( QFrame.StyledPanel | QFrame.Sunken )
            lyt.addWidget(lbl, row, col)

            col += 1
            if col%4 == 0:
                col = 0
                row += 1
        return main

#------------------------------------
# gui
#------------------------------------
class MainWidget(QWidget):
    def __init__(self):
        QWidget.__init__(self)
        self.lyt = QVBoxLayout()
        self.setLayout(self.lyt)
        
        #list of host & services
        self.table_main = QWidget()
        self.table_main_lyt = QVBoxLayout(self.table_main)
        self.info_table = QWidget()
        self.table_main_lyt.addWidget(self.info_table)
        self.lyt.addWidget(self.table_main)
        
        self.init_refresh()

        self.refresh()

    def init_refresh(self):
        self.refrsh_opt = QWidget()
        self.refrsh_opt_lyt = QHBoxLayout()
        self.refrsh_opt.setLayout(self.refrsh_opt_lyt)
        
        self.refrsh_btn = QPushButton()
        self.refrsh_btn.setIcon( self.style().standardIcon(QStyle.SP_BrowserReload)  )
        QObject.connect(self.refrsh_btn, SIGNAL('clicked()'), lambda: self.refresh)
        
        self.refrsh_btn.clicked.connect(self.refresh)
        self.refrsh_opt_lyt.addWidget(self.refrsh_btn)
        
        self.lyt.addWidget(self.refrsh_opt)

    def refresh(self):
        print("Signal: refresh")
        self.netinfo = get_host_summary()

        self.table_main_lyt.removeWidget( self.info_table )
        self.info_table = QWidget()
        self.info_table_lyt = QGridLayout(self.info_table)
        self.table_main_lyt.addWidget(self.info_table)

        row = 0
        col = 0
        for iHeadline in ("Interface", "Mac", "Ip", "Hostname") :
            self.info_table_lyt.addWidget(QLabel("<b>"+iHeadline+"</b>"), row, col ); col += 1
        for iServiceName, iService in SERVICES.items():
            self.info_table_lyt.addWidget(QLabel("<b>"+iServiceName+"</b>"), row, col ); col += 1

        row += 1
        col = 0
        for iHost in self.netinfo:
            self.info_table_lyt.addWidget(QLabel(str(iHost["dev"])), row, col ); col += 1
            self.info_table_lyt.addWidget(QLabel(str(iHost["mac"])), row, col ); col += 1
            self.info_table_lyt.addWidget(QLabel(str(iHost["ip"])), row, col );  col += 1
            
            if "hostname" in iHost:
                self.info_table_lyt.addWidget(QLabel(str(iHost["hostname"])), row, col );   
            else:
                self.info_table_lyt.addWidget(QLabel(""), row, col );   
            
            for iServiceName, iService in SERVICES.items():
                col += 1
                service_info = iHost["services"][iServiceName]
                if len(service_info) < 1:
                    self.info_table_lyt.addWidget(QLabel("x"), row, col )
                    continue
                widget = iService["display"].display(service_info)
                self.info_table_lyt.addWidget(widget, row, col )

            row += 1
            col = 0

if __name__ == '__main__':
    #service definitions

    SERVICES["smbserver"] = {"data": SmbHostService, "display":ServiceHelper}
    SERVICES["smb"] =       {"data": SmbService, "display":ServiceHelper}
    SERVICES["domain"] =    {"data": PortServie({"name": "domain",  "port": "53", "actions": []  }), "display":ServiceHelper}
    SERVICES["http"] =      {"data": PortServie({"name": "http",  "port": "80", "actions": []  }), "display":ServiceHelper}
    SERVICES["ssh"] =       {"data": PortServie({"name": "ssh",  "port": "22", "actions": []  }), "display":ServiceHelper}
    SERVICES["ftp"] =       {"data": PortServie({"name": "ftp",  "port": "21", "actions": []  }), "display":ServiceHelper}

    # Create the Qt Application
    app = QApplication(sys.argv)
   
    #maingui
    main = MainWidget()
    main.show()


    sys.exit(app.exec_())

