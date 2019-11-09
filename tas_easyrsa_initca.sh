#!/bin/bash
#
# Arguments
#   1. config name
#
#



SRV_CFG_NAME=$1
CLI_CFG_NAME="gamingclient"

CA_NAME="myca"
SRV_NAME="myserver"

#DO NOT EDIT
TARGET=$(pwd)/
CA_DIR=$TARGET/$CA_NAME



mkdir -p $CA_DIR/
cd $CA_DIR/
cp /etc/easy-rsa/openssl-easyrsa.cnf $CA_DIR/
cp -R /etc/easy-rsa/x509-types $CA_DIR/

export EASYRSA_CA_EXPIRE=2920
export EASYRSA_CERT_EXPIRE=2920
export EASYRSA_CRL_EXPIRE=2920
export EASYRSA_SSL_CONF=$CA_DIR/openssl-easyrsa.cnf 
export EASYRSA_ALGO=rsa
export EASYRSA_KEY_SIZE=2048


echo "[CA]"
cd $CA_DIR
export EASYRSA=$(pwd)
easyrsa init-pki
echo "set_var EASYRSA_REQ_CN         \"$CA_NAME\" " > vars
echo "set_var EASYRSA_BATCH          \"yes\" "      >> vars
easyrsa build-ca nopass

echo "set_var EASYRSA_REQ_CN         \"$SRV_NAME\" " > vars
echo "set_var EASYRSA_BATCH          \"yes\" "      >> vars
easyrsa gen-req $SRV_NAME nopass
easyrsa sign-req server ${SRV_NAME}
easyrsa gen-dh

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
cat $CA_DIR/pki/private/${SRV_NAME}.key     >> $TARGET/${SRV_CFG_NAME}.conf
echo "</key>"                               >> $TARGET/${SRV_CFG_NAME}.conf

echo "<dh>"                                 >> $TARGET/${SRV_CFG_NAME}.conf
cat $CA_DIR/pki/dh.pem                     >> $TARGET/${SRV_CFG_NAME}.conf
echo "</dh>"                                >> $TARGET/${SRV_CFG_NAME}.conf

