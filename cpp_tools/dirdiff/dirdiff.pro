QT += gui widgets

SOURCES =   main.cpp \
            filesys.cpp \
            treemodel.cpp

HEADERS = treemodel.h

CONFIG += debug

unix:LIBS += -lboost_system -lboost_filesystem 