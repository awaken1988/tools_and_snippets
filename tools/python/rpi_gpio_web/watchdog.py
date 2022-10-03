#!/usr/bin/env python
 
import RPi.GPIO as GPIO
import time


while True:
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(14, GPIO.OUT)
    GPIO.output(14, GPIO.LOW)
    time.sleep(10)
