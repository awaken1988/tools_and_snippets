#!/usr/bin/python

import os
import argparse
import subprocess
import shutil

ENV=dict(os.environ)
ENV["EASYRSA"]              = os.getcwd()
ENV["EASYRSA_CA_EXPIRE"]    = "2920"
ENV["EASYRSA_CERT_EXPIRE"]  = "2920"
ENV["EASYRSA_CRL_EXPIRE"]   = "2920"
ENV["EASYRSA_SSL_CONF"]     = os.getcwd()+"/openssl-easyrsa.cnf"
ENV["EASYRSA_ALGO"]         = "rsa" 
ENV["EASYRSA_KEY_SIZE"]     = "2048"

def paste_file(aDest, aSrc):
    for iLine in aSrc.readlines():
        aDest.write(iLine)


def action_init(args):
    print("init")
    print(args)
    shutil.copy("/etc/easy-rsa/openssl-easyrsa.cnf", "./openssl-easyrsa.cnf")
    shutil.copytree("/etc/easy-rsa/x509-types", "./x509-types")
    print(ENV)
    ENV["EASYRSA_REQ_CN"] = "myca" 
    subprocess.run(["easyrsa", "--batch", "init-pki"], env=ENV)
    subprocess.run(["easyrsa", "--batch", "build-ca", "nopass"], env=ENV)
    
    ENV["EASYRSA_REQ_CN"] = "myserver"
    subprocess.run(["easyrsa", "--batch", "gen-req", "myserver", "nopass"], env=ENV)
    subprocess.run(["easyrsa", "--batch", "sign-req", "server", "myserver"], env=ENV)
    subprocess.run(["easyrsa", "--batch",  "gen-dh"], env=ENV)

def action_add(args):
    print("add")
    print(args)
    ENV["EASYRSA_REQ_CN"] = args.name
    subprocess.run(["easyrsa", "--batch", "gen-req", args.name, "nopass"], env=ENV)
    subprocess.run(["easyrsa", "--batch", "sign-req", "client", args.name], env=ENV)
    
    f = open(args.name+".ovpn", "w")

    for iLine in args.config:
        f.write(iLine+"\n")

    f.write("<ca>\n")
    paste_file(f, open("pki/ca.crt"))
    f.write("</ca>\n")
    
    f.write("<cert>\n")
    paste_file(f, open("pki/issued/{}.crt".format(args.name)))
    f.write("</cert>\n")

    f.write("<key>\n")
    paste_file(f, open("pki/private/{}.key".format(args.name)))
    f.write("</key>\n")




parser      = argparse.ArgumentParser(prog="tas_easyrsa")
subparsers  = parser.add_subparsers()

init_parser = subparsers.add_parser("init", help="init ca")
init_parser.set_defaults(func=action_init)

add_parser = subparsers.add_parser("add", help="add new client config")
add_parser.add_argument("--name", required=True)
add_parser.add_argument("--config", nargs="*")
add_parser.set_defaults(func=action_add)

args = parser.parse_args()
args.func(args)

