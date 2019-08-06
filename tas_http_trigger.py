#!/usr/bin/env python
 
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib
import ipaddress
import subprocess
import sys

def trigger(aIpAddress):
  addr = ipaddress.ip_address(aIpAddress)
  try:
    if 4 == addr.version:
        subprocess.Popen("nft add element inet filter knocked {{ {}  }}".format(aIpAddress), shell=True)
    elif 6 == addr.version:
        subprocess.Popen("nft add element inet filter knocked6 {{ {}  }}".format(aIpAddress), shell=True)
  except:
    pass

# HTTPRequestHandler class
class testHTTPServer_RequestHandler(BaseHTTPRequestHandler):
 
  # GET
  def do_GET(self):
        message = 'trigger failed'
        get_dict = urllib.parse.parse_qs(self.path[2:])
        print(get_dict)
        if "secret" in get_dict and SECRET == get_dict["secret"][0]:
            client = self.client_address[0]

            if "ip" in get_dict:
                client = get_dict["ip"][0]

            ipaddress.ip_address(client)
            trigger(client)    
            message = 'trigger ok'
            
        self.send_response(200)
 
        self.send_header('Content-type','text/html')
        self.end_headers()
 
        self.wfile.write(bytes(message, "utf8"))
        return
 
def run():
  global PORT
  global SECRET
  PORT = sys.argv[1]
  SECRET = sys.argv[2]   

  print("PORT={}".format(PORT))
  print("SECRET={}".format(SECRET))

  server_address = ('', int(PORT) )
  httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
  httpd.serve_forever()
 
 
run()
