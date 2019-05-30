import subprocess
import json 
import socket
import sys
import time
from PySide2.QtWidgets  import (QLineEdit, QPushButton, QApplication, QVBoxLayout, QDialog, QTableView)
from PySide2.QtCore     import (QAbstractTableModel, QModelIndex, Qt)
from PySide2.QtWebEngineWidgets import (QWebEngineView)

#TODO name sheme for globals
web = None
host_list = None
lan_services = [
    {"ftp":     {"tcp_port": 21     }  },
    {"ssh":     {"tcp_port": 22     }  },
    {"dns":     {"tcp_port": 22     }  },
    {"http":    {"tcp_port": 80     }  },
    {"https":   {"tcp_port": 443    }  },
]
#------------------------------------------
# services to scan
#------------------------------------------


#------------------------------------------
# network helper
#------------------------------------------
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

WEBVIEW_CONTENT_SKELETON = """
<!DOCTYPE html>
<html>
    <head/>
        <title>Browse net services</title>
        <script>
            function hide_table() {
                var link = document.getElementById('content_table');
                link.style.display = 'none'; //or
                link.style.visibility = 'hidden';
            }
         
        </script>
    </head>
    <body>
        <table id="content_table">
            <tr>
                <th>Interface</th>
                <th>Ip</th>
                <th>Mac</th>
                <th>Hostname</th>
            </tr>
        </table>
        <p>test</p>
    </body>
</html>
"""


def fill_web_table():
    #initial
    host_list = get_neighbors()
    for iHost in host_list:
        query = "qt.jQuery('#content_table tr:last').after('"
        query += "<tr><td>{}</td><td>{}</td><td>{}</td><td>{}</td></tr>".format(
            iHost["dev"], 
            iHost["ip"], 
            iHost["mac"],
            get_hostname(iHost["ip"]))
        query += "');"
        web.page().runJavaScript(query)


def loadfinished(aIsOk):
    #load jQuery stuff
    with open("extern/jquery-min.js", "r") as jqFile:
        content = jqFile.read()
        content += "\nvar qt = { 'jQuery': jQuery.noConflict(true) };"
        web.page().runJavaScript(content)
    
    fill_web_table()
    
    web.page().runJavaScript("qt.jQuery('#content_table tr:last').after('<tr><td>bla2a</td></tr>');")

if __name__ == '__main__':
    # Create the Qt Application
    app = QApplication(sys.argv)

    web = QWebEngineView()
    web.setHtml(WEBVIEW_CONTENT_SKELETON)
    web.loadFinished.connect(loadfinished)
    
    # Run the main Qt loop
    web.show()
    sys.exit(app.exec_())

