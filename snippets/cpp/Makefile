CXXFLAGS= -g -O0 
OBJECTS := $(patsubst %.cpp,out/%, $(wildcard *.cpp))


.PHONY : all mkdir

all: mkdir | $(OBJECTS)

mkdir:
	-@mkdir out 2> /dev/null | true

out/%:	%.cpp
	g++ $(CXXFLAGS) $< -o $@

out/boost_fs_example:	boost_fs_example.cpp
	g++ -lboost_system -lboost_filesystem $(CXXFLAGS) $< -o $@