QT += gui widgets

SOURCES =   main.cpp \
            filesys.cpp \      
            fsdiff.cpp \    
           	treemodel.cpp \
           	sortfilterproxy.cpp \
           	maingui.cpp \
           	detailgui.cpp

HEADERS = 	treemodel.h \
			sortfilterproxy.h \
			maingui.h \
			detailgui.h

CONFIG += debug

unix:LIBS += -lboost_system -lboost_filesystem 