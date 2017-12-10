CXXFLAGS= -g -O0
OBJECTS := $(patsubst %.cpp,out/%, $(wildcard *.cpp))


.PHONY : all

all: $(OBJECTS)

out/%:	%.cpp
	g++ $(CXXFLAGS) $< -o $@