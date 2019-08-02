#TODO:
#   - add host specific services: e.g ping, nmap, speedtests
#
#
#
#
#

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
from PySide2.QtCore             import (QObject, QAbstractTableModel, QModelIndex, QUrl, Qt, Signal, Slot, SIGNAL)
from PySide2.QtWebEngineWidgets import (QWebEngineView, QWebEnginePage, QWebEngineProfile, QWebEngineScript)
from PySide2.QtWebChannel       import (QWebChannel)
from PySide2.QtWebSockets       import (QWebSocketServer)
from PySide2.QtNetwork          import (QHostAddress)
from PySide2.QtGui              import (QDesktopServices)

#include platform specific stuff
if platform.system() == "Linux":    
    from platform_linux import Platform
elif platform.system() == "Windows":
    from platform_windows import Platform
else:
    raise Exception("Your Platform not yet supported")

SERVICES = {}
EXECUTABLES = Platform.getPlatformExecutables()

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
        if Platform.which_command(aExecutable[iKey]["cmd"]):
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

    ret = Platform.get_hosts()

    #TODO: move this to tas_network_browser.py
    ret = list(filter(lambda x: not x["ip"].startswith("255.255.255.255"), ret))
    ret = list(filter(lambda x: not x["ip"].startswith("ff"), ret))
    ret = list(filter(lambda x: not x["mac"].startswith("00:00:00:00:00"), ret))
    for iMult4 in range(224, 239+1):
        ret = list(filter(lambda x: not x["ip"].startswith(str(iMult4)), ret))

    #append localhost
    ret.append( {   "dev":      "lo", 
                    "ip":       "127.0.0.1",
                    "mac":   "00:00:00:00:00:00"} )

    for i in ret:
        print(i)

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

def is_ipv6(aAddress):
    try:
        socket.inet_pton(socket.AF_INET6, aAddress)
    except e:
        print("geht nicht")
        return False
    return True

def addr_to_url(aAddress):
    if is_ipv6(aAddress):
        aAddress = "[" + aAddress + "]"
    return aAddress


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

    def add_action(self, aName, aAction):
        self.port_info["actions"].append( {"name": aName, "exec": aAction} )

class SmbHostService:
    @staticmethod
    def available(aExecutable):
        return True

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
    def available(aExecutable):
        return Platform.SmbService.available(aExecutable)

    @staticmethod
    def action_print1(aServiceInfo):
        print("1_mount "+ str(aServiceInfo))
    
    @staticmethod
    def action_print2(aServiceInfo):
        print("2_mount "+ str(aServiceInfo))

    @staticmethod
    def fetchinfo(aHostInfo):
        ret = []

        for iSmbShare in Platform.SmbService.fetchinfo(aHostInfo):
            ret.append(  {  "host":         aHostInfo["ip"],
                "display_name": iSmbShare,
                "actions":       [
                    {"name": "print_v1", "exec":  SmbService.action_print1},
                    {"name": "print_v2", "exec":  SmbService.action_print2},
                ],
                "smb_path":     [aHostInfo["ip"], iSmbShare]} )

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

    @staticmethod
    def open_default_browser(aUrl):


        QDesktopServices.openUrl(QUrl(aUrl, QUrl.TolerantMode))

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

    if SmbHostService.available(EXECUTABLES):
        SERVICES["smbserver"] = {"data": SmbHostService, "display":ServiceHelper}
    if SmbService.available(EXECUTABLES):
        SERVICES["smb"] =       {"data": SmbService, "display":ServiceHelper}
    SERVICES["domain"] =    {"data": PortServie({"name": "domain",  "port": "53", "actions": []  }), "display":ServiceHelper}
    SERVICES["http"] =      {"data": PortServie({"name": "http",  "port": "80", "actions": []  }), "display":ServiceHelper}
    SERVICES["https"] =     {"data": PortServie({"name": "https",  "port": "443", "actions": []  }), "display":ServiceHelper}
    SERVICES["ssh"] =       {"data": PortServie({"name": "ssh",  "port": "22", "actions": []  }), "display":ServiceHelper}
    SERVICES["ftp"] =       {"data": PortServie({"name": "ftp",  "port": "21", "actions": []  }), "display":ServiceHelper}

    #platform independent actions
    default_actions = [
        {"service": "http",        "name": "open default browser",      
        "action":  lambda aInfo: ServiceHelper.open_default_browser("http://"+addr_to_url(aInfo["host"]))},
        {"service": "https",       "name": "open default browser",      
        "action":  lambda aInfo: ServiceHelper.open_default_browser("https://"+addr_to_url(aInfo["host"]))},
    ]
    for iAction in default_actions:
        if iAction["service"] not in SERVICES:
            continue
        SERVICES[iAction["service"]]["data"].add_action(iAction["name"], iAction["action"])

    #platform dependet actions
    Platform.add_platform_actions(SERVICES)

    # Create the Qt Application
    app = QApplication(sys.argv)
   
    #maingui
    main = MainWidget()
    main.show()


    sys.exit(app.exec_())

