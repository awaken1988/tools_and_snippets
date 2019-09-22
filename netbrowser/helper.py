from PySide2.QtWidgets          import (QLineEdit, QPushButton, QApplication, 
                                        QVBoxLayout, QHBoxLayout, QDialog, QTableView, QGridLayout, QFormLayout,
                                        QLabel, QWidget, QAction, QMenu, QToolButton, QComboBox, QToolBar, QFrame, QSystemTrayIcon, QStyle)
from PySide2.QtCore             import (QObject, QAbstractTableModel, QModelIndex, QUrl, Qt, Signal, Slot, SIGNAL)
from PySide2.QtWebEngineWidgets import (QWebEngineView, QWebEnginePage, QWebEngineProfile, QWebEngineScript)
from PySide2.QtWebChannel       import (QWebChannel)
from PySide2.QtWebSockets       import (QWebSocketServer)
from PySide2.QtNetwork          import (QHostAddress)
from PySide2.QtGui              import (QDesktopServices)

def add_list(aDict, aKey, aElementToAdd):
    if aKey not in aDict:
        aDict[aKey] = []    
    aDict[aKey].append(aElementToAdd)

def get_dict_or_empty(aDict, aKey):
    if aKey not in aDict:
        return []
    return aDict[aKey]

#show name password dialog
class NamePasswordWidget(QDialog):

    def __init__(self, aFieldDefiniton):
        QDialog.__init__(self)

        self.setModal(True)

        self.field_defintion = aFieldDefiniton

        self.main_lyt = QVBoxLayout()
        self.setLayout(self.main_lyt)

        self.form_lyt = QFormLayout()
        self.input = {}
        for iField in self.field_defintion:
            curr_widget = QLineEdit()
            if "type" in iField and  "password" == iField["type"]:
                curr_widget.setEchoMode(QLineEdit.Password)
            self.form_lyt.addRow(iField["name"], curr_widget)
            self.input[iField["name"]] = curr_widget
        self.main_lyt.addLayout(self.form_lyt)

        self.btn_ok = QPushButton("Ok")
        self.btn_ok.clicked.connect(lambda: self.accept())
        
        self.btn_cancel = QPushButton("Cancel")
        self.btn_cancel.clicked.connect(lambda: self.reject())
        
        self.btn_lyt = QHBoxLayout()
        self.btn_lyt.addWidget(self.btn_ok)
        self.btn_lyt.addWidget(self.btn_cancel)
        self.main_lyt.addLayout(self.btn_lyt)

    def getData(self, aName):
        return self.input[aName].text()