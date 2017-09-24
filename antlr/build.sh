#!/bin/bash


ANTLR_PYTHON="antlr_py"
ANTLR_JAVA="antlr_java"

export CLASSPATH="./antlr-4.7-complete.jar:${ANTLR_JAVA}/*"

rm -rf ${ANTLR_PYTHON}
rm -rf ${ANTLR_JAVA}
mkdir ${ANTLR_PYTHON}
mkdir ${ANTLR_JAVA}

java -jar antlr-4.7-complete.jar -Dlanguage=Python3 -o ${ANTLR_PYTHON} demo.g4
java -jar antlr-4.7-complete.jar -o ${ANTLR_JAVA} demo.g4

javac ${ANTLR_JAVA}/demo*.java

cp antlr-4.7-complete.jar ${ANTLR_JAVA}
cd ${ANTLR_JAVA}
java -cp antlr-4.7-complete.jar:. org.antlr.v4.gui.TestRig demo addition -gui