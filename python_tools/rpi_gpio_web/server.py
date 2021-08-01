#!/usr/bin/env python
 
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib
import ipaddress
import subprocess
import sys
import socket
import RPi.GPIO as GPIO

def trigger(aIpAddress):
  addr = ipaddress.ip_address(aIpAddress)
  try:
    if 4 == addr.version:
        subprocess.Popen("nft add element inet filter knocked {{ {}  }}".format(aIpAddress), shell=True)
    elif 6 == addr.version:
        subprocess.Popen("nft add element inet filter knocked6 {{ {}  }}".format(aIpAddress), shell=True)
  except:
    pass

def mainpage():
    return open("index.html", mode="r").read() 

# HTTPRequestHandler class
class RequestHandler(BaseHTTPRequestHandler):
 
  # GET
  def do_GET(self):
        message = mainpage()
        opt = urllib.parse.parse_qs(self.path[2:])
        self.send_response(200)

        is_pump = "pump" in opt
        is_on   = is_pump and opt["pump"][0] == "on"
        is_off  = is_pump and opt["pump"][0] == "off"

        if is_on:
            GPIO.output(14, GPIO.HIGH)
        elif is_off:
            GPIO.output(14, GPIO.LOW)

        self.send_header('Content-type','text/html')
        self.end_headers()
 
        self.wfile.write(bytes(message, "utf8"))
        return

class HTTPServer6(HTTPServer):
  address_family = socket.AF_INET6

def run():
  global PORT
  global IPV 

  IPV = sys.argv[1]
  PORT = sys.argv[2]

  print("IPV={}".format(IPV))
  print("PORT={}".format(PORT))
    
  if "ip4" == IPV:
    server_address = ('', int(PORT) )
    httpd = HTTPServer(server_address, RequestHandler )
    httpd.serve_forever()
  elif "ip6" == IPV:
    server_address = ('::', int(PORT) )
    httpd = HTTPServer6(server_address, RequestHandler )
    httpd.serve_forever()

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BCM)
GPIO.setup(14, GPIO.OUT)
run()
