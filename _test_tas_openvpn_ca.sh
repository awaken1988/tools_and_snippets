#!/bin/bash
#References:
#	- https://gist.github.com/fntlnz/cf14feb5a46b2eda428e000157447309


KEYSIZE=1024



if [ ! -d ./ca ]; then
        mkdir ca

        openssl genrsa 	-out ca/root_ca.key 4096
        openssl req    -x509 -new -nodes -key ca/root_ca.key \
               -sha256 -days 2048 -out ca/root_ca.crt \
	       -subj "/C=DE/ST=myca_st/L=myca_l/O=myca_o/CN=myca_cn"
        openssl dhparam -out ca/root_ca_dh.pem $KEYSIZE
fi



if [ ! -d ./server ]; then
	mkdir server
	openssl genrsa -out ./server/server.key
	openssl req -new -sha256 -key ./server/server.key \
                -subj "/C=DE/ST=server_st/O=server_o/CN=server_cn" \
		-out ./server/server.csr
	openssl x509 -req -in ./server/server.csr \
		-CA ./ca/root_ca.crt -CAkey ./ca/root_ca.key -CAcreateserial \
		-out server/server.crt -days 2048 -sha256
	#---------------------------------------------------
	# OpenVPN server config
	#---------------------------------------------------
	echo "[GENERATE_SERVER_CONFIG]"
	echo "port 1194"                            >  ./server/server.conf
	echo "proto udp"                            >> ./server/server.conf
	echo "dev $SRV_CFG_NAME"                    >> ./server/server.conf
	echo "dev-type tap"                         >> ./server/server.conf
	echo "keepalive 10 120"                     >> ./server/server.conf
	echo "server 192.168.158.0 255.255.255.0"   >> ./server/server.conf

	echo "<ca>"                                 >> ./server/server.conf
	cat ./ca/root_ca.crt                        >> ./server/server.conf
	echo "</ca>"                                >> ./server/server.conf

	echo "<cert>"                               >> ./server/server.conf
	cat ./server/server.crt      		    >> ./server/server.conf
	echo "</cert>"                              >> ./server/server.conf

	echo "<key>"                                >> ./server/server.conf
	cat ./server/server.key                     >> ./server/server.conf
	echo "</key>"                               >> ./server/server.conf

	echo "<dh>"                                 >> ./server/server.conf
	cat ./ca/root_ca_dh.pem                     >> ./server/server.conf
	echo "</dh>"                                >> ./server/server.conf
fi

if [ ! -d ./client ]; then
	mkdir ./client
fi



CLIENT_KEY=./client/$1.key
CLIENT_REQ=./client/$1.req
CLIENT_CRT=./client/$1.crt

openssl req 	-utf8 -new -newkey rsa:$KEYSIZE \
		-keyout $CLIENT_KEY \
		-out $CLIENT_REQ

openssl ca 	-utf8 \
		-in $CLIENT_REQ \
		-out $CLIENT_CRT \
		-days 1460 \
		-batch




