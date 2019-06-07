#References
#   Qt:
#       https://stackoverflow.com/questions/28565254/how-to-use-qt-webengine-and-qwebchannel
#       https://doc.qt.io/qt-5/qtwebchannel-javascript.html
#       https://stackoverflow.com/questions/31928444/qt-qwebenginepagesetwebchannel-transport-object
#       https://code.woboq.org/qt5/qtwebchannel/examples/webchannel/shared/websocketclientwrapper.cpp.html https://doc.qt.io/qt-5/qtwebchannel-standalone-main-cpp.html

import subprocess
import json 
import socket
import sys
import time
import re
from PySide2.QtWidgets          import (QLineEdit, QPushButton, QApplication, 
                                        QVBoxLayout, QHBoxLayout, QDialog, QTableView, QGridLayout, 
                                        QLabel, QWidget, QAction, QMenu, QToolButton,
                                        QComboBox, QToolBar, QFrame)
from PySide2.QtCore             import (QObject, QAbstractTableModel, QModelIndex, Qt, Signal, Slot)
from PySide2.QtWebEngineWidgets import (QWebEngineView, QWebEnginePage, QWebEngineProfile, QWebEngineScript)
from PySide2.QtWebChannel       import (QWebChannel)
from PySide2.QtWebSockets       import (QWebSocketServer)
from PySide2.QtNetwork          import (QHostAddress)

SERVICES = {}

#------------------------------------
# net helper
#------------------------------------
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

def fill_web_table(aInfoTable: QGridLayout):
    #initial
    host_list = get_neighbors()

    row = 0
    col = 0
    for iHeadline in ("Interface", "Mac", "Ip", "Hostname") :
        aInfoTable.addWidget(QLabel("<b>"+iHeadline+"</b>"), row, col ); col += 1
    for iServiceName, iService in SERVICES.items():
         aInfoTable.addWidget(QLabel("<b>"+iServiceName+"</b>"), row, col ); col += 1

    row += 1
    for iHost in host_list:
        #smb_shares = ""
        #for iSmbShares in get_smb_shares(iHost["ip"]):
        #    smb_shares += "\\\\\\\\"+iSmbShares[0]+"\\\\"+iSmbShares[1] + "<br>"

        hostname = get_hostname(iHost["ip"])
        
        col = 0

        aInfoTable.addWidget(QLabel(str(iHost["dev"])), row, col ); col += 1
        aInfoTable.addWidget(QLabel(str(iHost["mac"])), row, col ); col += 1
        aInfoTable.addWidget(QLabel(str(iHost["ip"])), row, col );  col += 1
        aInfoTable.addWidget(QLabel(str(hostname)), row, col );   
        
        for iServiceName, iService in SERVICES.items():
            col += 1
            info = iService["data"].fetchinfo(iHost)
            if len(info) < 1:
                aInfoTable.addWidget(QLabel("x"), row, col )
                continue
            widget = iService["display"].display(info)
            aInfoTable.addWidget(widget, row, col )

        row += 1

if __name__ == '__main__':
    #service definitions
    SERVICES["smb"] =       {"data": SmbService, "display":ServiceHelper}
    SERVICES["domain"] =    {"data": PortServie({"name": "domain",  "port": "53", "actions": []  }), "display":ServiceHelper}
    SERVICES["http"] =      {"data": PortServie({"name": "http",  "port": "80", "actions": []  }), "display":ServiceHelper}
    SERVICES["ssh"] =       {"data": PortServie({"name": "ssh",  "port": "22", "actions": []  }), "display":ServiceHelper}
    SERVICES["ftp"] =       {"data": PortServie({"name": "ftp",  "port": "21", "actions": []  }), "display":ServiceHelper}


    # Create the Qt Application
    app = QApplication(sys.argv)

    info_table = QWidget()
    lyt = QGridLayout(info_table)
    info_table.setLayout(lyt)


    fill_web_table(lyt)
    info_table.show()

    sys.exit(app.exec_())

