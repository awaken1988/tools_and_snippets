QT += gui widgets

SOURCES =   main.cpp \
            filesys.cpp \      
            fsdiff.cpp \    
           	treemodel.cpp \
           	sortfilterproxy.cpp

HEADERS = 	treemodel.h \
			sortfilterproxy.h

CONFIG += debug

unix:LIBS += -lboost_system -lboost_filesystem 