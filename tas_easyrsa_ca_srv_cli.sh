#!/bin/bash

SRV_CFG_NAME="gaming"
CLI_CFG_NAME="gamingclient"

CA_NAME="myca"
SRV_NAME="myserver"
CLI_NAME="myclient"


#DO NOT EDIT
TARGET=$(pwd)/gen
CA_DIR=$TARGET/$CA_NAME
SRV_DIR=$TARGET/$SRV_NAME
CLI_DIR=$TARGET/$CLI_NAME
ALL_PKI="$CA_DIR $SRV_DIR $CLI_DIR"
SRV_CLI_DIRS="$SRV_DIR $CLI_DIR"


for iPki in $ALL_PKI; do
    mkdir -p $iPki
    cp /etc/easy-rsa/openssl-easyrsa.cnf $iPki/
    cp -R /etc/easy-rsa/x509-types $iPki/

done;

echo "[CA]"
cd $CA_DIR
export EASYRSA=$(pwd)
easyrsa init-pki
echo "set_var EASYRSA_REQ_CN         \"$CA_NAME\" " >> vars
echo "set_var EASYRSA_BATCH          \"yes\" "  >> vars
easyrsa build-ca nopass

echo "[$CLI_NAME]"
cd $CLI_DIR
export EASYRSA=$(pwd)
easyrsa init-pki
echo "set_var EASYRSA_REQ_CN         \"$CLI_NAME\" " >> vars
echo "set_var EASYRSA_BATCH          \"yes\" "  >> vars
easyrsa gen-req $CLI_NAME

echo "[$SRV_NAME]"
cd $SRV_DIR
export EASYRSA=$(pwd)
easyrsa init-pki
echo "set_var EASYRSA_REQ_CN         \"$SRV_NAME\" " >> vars
echo "set_var EASYRSA_BATCH          \"yes\" "  >> vars
easyrsa gen-req $SRV_NAME nopass
easyrsa gen-dh

echo "[Sign]"
cd $CA_DIR
export EASYRSA=$(pwd)
easyrsa import-req $SRV_DIR/pki/reqs/${SRV_NAME}.req ${SRV_NAME}
easyrsa sign-req server ${SRV_NAME}

easyrsa import-req $CLI_DIR/pki/reqs/${CLI_NAME}.req ${CLI_NAME}
easyrsa sign-req client ${CLI_NAME}

#---------------------------------------------------
# OpenVPN server config
#---------------------------------------------------
echo "[GENERATE_SERVER_CONFIG]"           
echo "port 1194"                            >  $TARGET/${SRV_CFG_NAME}.conf
echo "proto udp"                            >> $TARGET/${SRV_CFG_NAME}.conf
echo "dev $SRV_CFG_NAME"                    >> $TARGET/${SRV_CFG_NAME}.conf
echo "dev-type tap"                         >> $TARGET/${SRV_CFG_NAME}.conf
echo "keepalive 10 120"                     >> $TARGET/${SRV_CFG_NAME}.conf
echo "server 192.168.158.0 255.255.255.0"   >> $TARGET/${SRV_CFG_NAME}.conf

echo "<ca>"                                 >> $TARGET/${SRV_CFG_NAME}.conf
cat $CA_DIR/pki/ca.crt                      >> $TARGET/${SRV_CFG_NAME}.conf
echo "</ca>"                                >> $TARGET/${SRV_CFG_NAME}.conf

echo "<cert>"                               >> $TARGET/${SRV_CFG_NAME}.conf
cat $CA_DIR/pki/issued/${SRV_NAME}.crt      >> $TARGET/${SRV_CFG_NAME}.conf
echo "</cert>"                              >> $TARGET/${SRV_CFG_NAME}.conf

echo "<key>"                                >> $TARGET/${SRV_CFG_NAME}.conf
cat $SRV_DIR/pki/private/${SRV_NAME}.key    >> $TARGET/${SRV_CFG_NAME}.conf
echo "</key>"                               >> $TARGET/${SRV_CFG_NAME}.conf

echo "<key>"                                >> $TARGET/${SRV_CFG_NAME}.conf
cat $SRV_DIR/pki/private/${SRV_NAME}.key    >> $TARGET/${SRV_CFG_NAME}.conf
echo "</key>"                               >> $TARGET/${SRV_CFG_NAME}.conf

echo "<dh>"                                 >> $TARGET/${SRV_CFG_NAME}.conf
cat $SRV_DIR/pki/dh.pem                     >> $TARGET/${SRV_CFG_NAME}.conf
echo "</dh>"                                >> $TARGET/${SRV_CFG_NAME}.conf

#---------------------------------------------------
# OpenVPN server config
#---------------------------------------------------
echo "[GENERATE_CLIENT_CONFIG]"           

echo "remote ip port"                   >  $TARGET/${CLI_CFG_NAME}.ovpn
echo "client"                               >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "dev tap"                              >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "<ca>"                                 >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CA_DIR/pki/ca.crt                      >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</ca>"                                >> $TARGET/${CLI_CFG_NAME}.ovpn

echo "<cert>"                               >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CA_DIR/pki/issued/${CLI_NAME}.crt      >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</cert>"                              >> $TARGET/${CLI_CFG_NAME}.ovpn

echo "<key>"                                >> $TARGET/${CLI_CFG_NAME}.ovpn
cat $CLI_DIR/pki/private/${CLI_NAME}.key    >> $TARGET/${CLI_CFG_NAME}.ovpn
echo "</key>"                               >> $TARGET/${CLI_CFG_NAME}.ovpn
