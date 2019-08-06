#!/usr/bin/env python
 
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib
import ipaddress
import subprocess

SECRET = 'asdf1234'

def trigger(aIpAddress):
    subprocess.Popen("echo "+aIpAddress+" ", shell=True)

# HTTPRequestHandler class
class testHTTPServer_RequestHandler(BaseHTTPRequestHandler):
 
  # GET
  def do_GET(self):
        get_dict = urllib.parse.parse_qs(self.path[2:])
        print(get_dict)
        if "secret" in get_dict and SECRET == get_dict["secret"][0]:
            client = self.client_address[0]

            if "ip" in get_dict:
                client = get_dict["ip"][0]

            try:
                ipaddress.ip_address(client)
                print("trigger")
                trigger(aIpAddress)    
            except:
                pass
            
            

            

        # Send response status code
        self.send_response(200)
 
        # Send headers
        self.send_header('Content-type','text/html')
        self.end_headers()
 
        # Send message back to client
        message = "Hello world!"
        # Write content as utf-8 data
        self.wfile.write(bytes(message, "utf8"))
        return
 
def run():
  server_address = ('127.0.0.1', 8081)
  httpd = HTTPServer(server_address, testHTTPServer_RequestHandler)
  httpd.serve_forever()
 
 
run()