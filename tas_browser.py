import subprocess
import json 
import socket
import sys
from PySide2.QtWidgets  import (QLineEdit, QPushButton, QApplication, QVBoxLayout, QDialog, QTableView)
from PySide2.QtCore     import (QAbstractTableModel, QModelIndex, Qt)


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

def print_outable(aTable):
    col_sizes = []
    for iLineIdx, iLine in enumerate(aTable):
        for iColIdx, iCol in enumerate(iLine):
            if iColIdx >= len(col_sizes):
                col_sizes.append( len(iCol) )
            else:
                if col_sizes[iColIdx] < len(iCol):
                    col_sizes[iColIdx] = len(iCol)
    
    for iLineIdx, iLine in enumerate(aTable):
        print(" ", end="")
        for iColIdx, iCol in enumerate(iLine):
            placeholder="{{: >{}}}".format(col_sizes[iColIdx])
            print(placeholder.format(iCol), end="   |   ")
        print("")



#a = get_neighbors()
#
#outtable = []
#for iEntry in a:
#    line = []
#    line.append(iEntry["dev"])
#    line.append(iEntry["ip"])
#
#    hostname = ""
#    try:
#        hostname = str(socket.gethostbyaddr(iEntry["ip"])[0])
#    except: 
#        pass
#    line.append(hostname)
#
#    for iPort in ("139", "21", "80"):
#        if scan_a_port(iEntry["ip"], iPort):
#            line.append(iPort)
#        else:
#            line.append("")
#
#    outtable.append(line)
#
#print_outable(outtable)

class HostModel(QAbstractTableModel):

    def __init__(self, aHostList, aColGetter, parent=None):
        super(HostModel, self).__init__(parent)

        self.host_list = aHostList
        self.col_getter = aColGetter

    def rowCount(self, index=QModelIndex()):
        """ Returns the number of rows the model holds. """
        return len(self.col_getter)

    def columnCount(self, index=QModelIndex()):
        """ Returns the number of columns the model holds. """
        return len(self.col_getter)

    def data(self, index, role=Qt.DisplayRole):
        """ Depending on the index and role given, return data. If not 
            returning data, return None (PySide equivalent of QT's 
            "invalid QVariant").
        """
        if not index.isValid():
            return None

        if not 0 <= index.row() < len(self.host_list):
            return None

        if role == Qt.DisplayRole:
            return ["a", "b"]
            #return self.col_getter[index.column()]["Getter"](self.host_list[index.row()])

        return None

    def headerData(self, section, orientation, role=Qt.DisplayRole):
        """ Set the headers to be displayed. """
        if role != Qt.DisplayRole:
            return None

        if orientation == Qt.Horizontal:
            return self.col_getter[section]["Name"]
        
        return None

    def insertRows(self, position, rows=1, index=QModelIndex()):
        #""" Insert a row into the model. """
        #self.beginInsertRows(QModelIndex(), position, position + rows - 1)

        #for row in range(rows):
        #    self.hostinfo.insert(position + row, {"name":"", "address":""})

        #self.endInsertRows()
        #return True
        return False

    def removeRows(self, position, rows=1, index=QModelIndex()):
        #""" Remove a row from the model. """
        #self.beginRemoveRows(QModelIndex(), position, position + rows - 1)

        #del self.hostinfo[position:position+rows]

        #self.endRemoveRows()
        #return True
        return False

    def setData(self, index, value, role=Qt.EditRole):
        #""" Adjust the data (set it to <value>) depending on the given 
        #    index and role. 
        #"""
        #if role != Qt.EditRole:
        #    return False

        #if index.isValid() and 0 <= index.row() < len(self.hostinfo):
        #    address = self.hostinfo[index.row()]
        #    if index.column() == 0:
        #        address["name"] = value
        #    elif index.column() == 1:
        #        address["address"] = value
        #    else:
        #        return False

        #    self.dataChanged.emit(index, index)
        #    return True

        return False

    def flags(self, index):
        """ Set the item flags at the given index. Seems like we're 
            implementing this function just to see how it's done, as we 
            manually adjust each tableView to have NoEditTriggers.
        """
        if not index.isValid():
            return Qt.ItemIsEnabled
        return Qt.ItemFlags(QAbstractTableModel.flags(self, index) |
Qt.ItemIsEditable)



if __name__ == '__main__':
    # Create the Qt Application
    app = QApplication(sys.argv)

    #get all ip adresses in the lan
    host_list = get_neighbors()
    print(host_list)

    col_getter = [  {"Name": "Dev", "Getter": lambda aHostItem: aHostItem["dev"]},
                    {"Name": "Ip", "Getter": lambda aHostItem: aHostItem["ip"]},
                    {"Name": "Mac", "Getter": lambda aHostItem: aHostItem["mac"]}, ]


    host_table = QTableView()
    host_model = HostModel(host_list, col_getter, host_table)
    host_table.setModel(host_model)
    host_table.show()

    # Run the main Qt loop
    sys.exit(app.exec_())




















    #class Form(QDialog):
#
#    def __init__(self, parent=None):
#        super(Form, self).__init__(parent)
#        # Create widgets
#        self.edit = QLineEdit("Write my name here")
#        self.button = QPushButton("Show Greetings")
#        # Create layout and add widgets
#        layout = QVBoxLayout()
#        layout.addWidget(self.edit)
#        layout.addWidget(self.button)
#        # Set dialog layout
#        self.setLayout(layout)
#        # Add button signal to greetings slot
#        self.button.clicked.connect(self.greetings)
#
#    # Greets the user
#    def greetings(self):
#        print ("Hello %s" % self.edit.text())