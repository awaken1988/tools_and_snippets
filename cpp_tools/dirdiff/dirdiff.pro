QT += gui widgets

SOURCES =   main.cpp \     
            fsdiff.cpp \    
           	treemodel.cpp \
           	sortfilterproxy.cpp \
           	maingui.cpp \
           	detailgui.cpp \
           	opengui.cpp

HEADERS = 	treemodel.h \
			sortfilterproxy.h \
			maingui.h \
			detailgui.h \
			opengui.h

CONFIG += debug

unix:LIBS += -lboost_system -lboost_filesystem 

QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-parameter