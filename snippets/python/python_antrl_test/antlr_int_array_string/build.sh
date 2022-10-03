#!/bin/bash


ANTLR_PYTHON="build_py"
ANTLR_JAVA="build_java"

export CLASSPATH="./antlr-4.7-complete.jar:${ANTLR_JAVA}/*"

rm -rf ${ANTLR_PYTHON}
rm -rf ${ANTLR_JAVA}
mkdir ${ANTLR_PYTHON}
mkdir ${ANTLR_JAVA}

if [ ! -e "antlr-4.7-complete.jar" ]; then
    echo "antlr not found -> download it"
    wget http://www.antlr.org/download/antlr-4.7-complete.jar
fi

java -jar antlr-4.7-complete.jar -visitor -Dlanguage=Python3 -o ${ANTLR_PYTHON} demo.g4
java -jar antlr-4.7-complete.jar -visitor -o ${ANTLR_JAVA} demo.g4

javac ${ANTLR_JAVA}/demo*.java


if [ "$#" -gt 1 ] || [ "$1" == "testrig" ]; then
    echo "compile ready: starting TestRig"
    cp antlr-4.7-complete.jar ${ANTLR_JAVA}
    cd ${ANTLR_JAVA}
    java -cp antlr-4.7-complete.jar:. org.antlr.v4.gui.TestRig demo init -gui
fi