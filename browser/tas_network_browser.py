#References
#   Qt:
#       https://stackoverflow.com/questions/28565254/how-to-use-qt-webengine-and-qwebchannel
#       https://doc.qt.io/qt-5/qtwebchannel-javascript.html
#       https://stackoverflow.com/questions/31928444/qt-qwebenginepagesetwebchannel-transport-object
#       https://code.woboq.org/qt5/qtwebchannel/examples/webchannel/shared/websocketclientwrapper.cpp.html https://doc.qt.io/qt-5/qtwebchannel-standalone-main-cpp.html


import mod_net
import subprocess
import json 
import socket
import sys
import time
from PySide2.QtWidgets          import (QLineEdit, QPushButton, QApplication, QVBoxLayout, QDialog, QTableView)
from PySide2.QtCore             import (QObject, QAbstractTableModel, QModelIndex, Qt, __version_info__, Signal, Slot)
from PySide2.QtWebEngineWidgets import (QWebEngineView, QWebEnginePage)
from PySide2.QtWebChannel       import (QWebChannel)
from PySide2.QtWebSockets       import (QWebSocketServer)
from PySide2.QtNetwork          import (QHostAddress)

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


WEBVIEW_CONTENT_SKELETON = """
<!DOCTYPE html>
<html>
    <head/>
        <title>Browse net services</title>
        <script type="text/javascript" src="qrc:///qtwebchannel/qwebchannel.js"></script>
        <script type="text/javascript">
            function hide_table() {
                var link = document.getElementById('content_table');
                link.style.display = 'none'; //or
                link.style.visibility = 'hidden';
            }

            __replace_this_with_all_javascript_library_stuff__
         
            //QWebChannel doesnt work because transport send doesnt exist
            //document.addEventListener("DOMContentLoaded", function (){
            //    window.mywebchannel = new QWebChannel(qt.webChannelTransport, function (channel) {
            //        //channel.objects.MyChannelHanlder.message_from_web()
            //        console.log("bla")
            //    });
            //});

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
        <a href="http://www.google.de">blabla</a>
    </body>
</html>
"""

def fill_web_table():
    #initial
    host_list = mod_net.get_neighbors()



    for iHost in host_list:
        smb_shares = ""
        for iSmbShares in mod_net.get_smb_shares(iHost["ip"]):
            smb_shares += "\\\\\\\\"+iSmbShares[0]+"\\\\"+iSmbShares[1] + "<br>"

        hostname = mod_net.get_hostname(iHost["ip"])
        
        query = "qt.jQuery('#content_table tr:last').after('<tr>"
        query += "<td>{}</td>".format(iHost["dev"])
        query += "<td>{}</td>".format(iHost["ip"])
        query += "<td>{}</td>".format(iHost["mac"])
        query += "<td>{}</td>".format(hostname)
        query += "<td>{}</td>".format(smb_shares)
        query += "</tr>');"
        web.page().runJavaScript(query)


def loadfinished(aIsOk):
    fill_web_table()
    
    web.page().runJavaScript("qt.jQuery('#content_table tr:last').after('<tr><td>bla2a</td></tr>');")

if __name__ == '__main__':
    print(__version_info__)

    


    # Create the Qt Application
    app = QApplication(sys.argv)

    #load all javascript libraries
    with open("extern/jquery-min.js", "r") as jqFile, open("extern/qwebchannel.js") as qwFile:
        content =  "\n" + jqFile.read()
        #content += "\n" + qwFile.read()
        content += "\n" + "var qt = { 'jQuery': jQuery.noConflict(true) };"
        WEBVIEW_CONTENT_SKELETON = WEBVIEW_CONTENT_SKELETON.replace(" __replace_this_with_all_javascript_library_stuff__", content)

    #socket_server = QWebSocketServer("QWebSocketServer workaround server", QWebSocketServer.NonSecureMode)##
    #if not socket_server.listen(QHostAddress.LocalHost, 12345):
    #    raise Exception("uh cannot create socket server")
  

   
    web = QWebEngineView()
    web.setHtml(WEBVIEW_CONTENT_SKELETON)
    web.loadFinished.connect(loadfinished)
    
    # Run the main Qt loop
    web.show()
    sys.exit(app.exec_())

