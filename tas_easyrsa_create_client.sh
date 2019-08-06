#!/bin/bash

#
# Arguments
#   1. config name
#   2. ip or hostname
#   3. port

CA_NAME="myca"
SRV_NAME="myserver"

#DO NOT EDIT
TARGET=$(pwd)/gen
CA_DIR=$TARGET/$CA_NAME

cd $CA_DIR/

CLI_NAME="client_$1"
CLI_CFG_NAME="$CLI_NAME"

echo "set_var EASYRSA_REQ_CN         \"$CLI_NAME\" " > vars
echo "set_var EASYRSA_BATCH          \"yes\" "      >> vars
easyrsa gen-req $CLI_NAME nopass
easyrsa sign-req client $CLI_NAME

echo "[GENERATE_CLIENT_CONFIG]"           

echo "remote $2 $3"                         >  $TARGET/${CLI_CFG_NAME}.ovpn
echo "client"                               >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "dev tap"                              >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "<ca>"                                 >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CA_DIR/pki/ca.crt                      >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</ca>"                                >> $TARGET/${CLI_CFG_NAME}.ovpn

echo "<cert>"                               >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CA_DIR/pki/issued/${CLI_NAME}.crt      >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</cert>"                              >> $TARGET/${CLI_CFG_NAME}.ovpn

echo "<key>"                                >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CA_DIR//pki/private/${CLI_NAME}.key    >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</key>"                               >> $TARGET/${CLI_CFG_NAME}.ovpn



