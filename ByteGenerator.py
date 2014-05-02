'''
Created on 2 May 2014

@author: martin
'''
import random


def generateRandom(aStream, aSizeByte):
    random.seed()    
    for i in range(0, aSizeByte):
        currVal = bytes([random.randrange(0, 256)])
        aStream.write(currVal)
        
def generateCounter(aStream, aSizeByte, aStart, aIncLen):
    random.seed()
    
    currIncLen = 0
    for i in range(0, aSizeByte):
        if aStart > 255: aStart = 0;
        currVal = bytes([aStart])
        aStream.write(currVal)
        currIncLen = currIncLen + 1
        if currIncLen >= aIncLen:
            currIncLen = 0
            aStart     = aStart + 1;


            
            
        

myFile = open("test.bin", "wb")        

generateCounter(myFile, 256, 3, 4)