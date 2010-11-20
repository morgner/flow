#! /usr/bin/env bash

#
#  create-set-of-certificates-hierarchical.sh 
#
#  Copyright (C) 2010 by Manfred Morgner
#  manfred@morgner.com
#
#  This script creates a bunch of certifictes to work with for
#  testing and/or developing (on) the 'flow' system
#  ---
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  ---
#
#  Test a certificate
#  ------------------
#  openssl verify -CAfile <ca-chain-file> <cert-name>.crt
#
#  Construct a TLS secured test connection
#  ---------------------------------------
#  openssl s_server -accept 30000 \
#                   -cert server/localhost.crt -key server/localhost.key -CAfile server/server-CA-chain.pem -tls1
#
#  openssl s_client -connect localhost:30000 \
#                   -cert client/username.crt -key client/username.key -CAfile client/client-CA-chain.pem -tls1 -verify 3


RTCA="cert.flow-ca.info"
ISCA="Intermediate-Server-CA"
ICCA="Intermediate-Client-CA"

FLOWSERVER="localhost"
FLOWSORGAN="'flow' Working Group"
FLOWSCOUNT="CH"

USER1="username"
USER2="recipient"


WORKDIR=`pwd`
README=${WORKDIR}/CERTIFICATES
rm -f ${README}

mkdir -p CA
mkdir -p client
mkdir -p server

DIR_CA=${WORKDIR}/CA
DIR_CLIENT=${WORKDIR}/client
DIR_SERVER=${WORKDIR}/server

CLIENT_CA_CHAIN=${DIR_CLIENT}/client-CA-chain.pem
SERVER_CA_CHAIN=${DIR_SERVER}/server-CA-chain.pem


#
# Root CA
#
echo "Creating Root CA"

cd ${DIR_CA}

NAME=${RTCA}
ORGANISATION="Great Privacy Commitee"
COUNTRY="CH"
STATE="Bern"
LOCATION="Stadt Bern"
openssl req -new -utf8 -x509 -days 7302 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.crt
echo "Root CA Certificate is: `pwd`/${NAME}.crt" | tee -a ${README}

#
# Intermediate Server CA
#
echo "Creating Intermediate Server CA"

NAME=${ISCA}
CN="Intermediate Server CA"
ORGANISATION="Great Privacy Server Management"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="100"
CA=${RTCA}
openssl x509 -req -days 7300 -extfile ${WORKDIR}/v3_ca.conf -extensions v3_ca -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
echo "Intermediate Server CA Certificate is: `pwd`/${NAME}.crt" | tee -a ${README}


#
# Intermediate Client CA
#
echo "Creating Intermediate Client CA"

NAME=${ICCA}
CN="Intermediate Client CA"
ORGANISATION="Great Privacy Client Administration"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="200"
CA=${RTCA}
openssl x509 -req -days 7300 -extfile ${WORKDIR}/v3_ca.conf -extensions v3_ca -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
echo "Intermediate Client CA Certificate is: `pwd`/${NAME}.crt" | tee -a ${README}


#
# Compose the CA Chains
#
echo "Compose CA Chains"

rm -f ${SERVER_CA_CHAIN}
for i in ${RTCA}.crt ${ISCA}.crt
  do
  openssl x509 -in $i -text >> ${SERVER_CA_CHAIN}
  done
echo "Server CA Chain is: ${SERVER_CA_CHAIN}" | tee -a ${README}

rm -f ${CLIENT_CA_CHAIN}
for i in ${RTCA}.crt ${ICCA}.crt
  do
  openssl x509 -in $i -text >> ${CLIENT_CA_CHAIN}
  done
echo "Client CA Chain is: ${CLIENT_CA_CHAIN}" | tee -a ${README}


#
# Server Certificate for ${FLOWSERVER}, ${FLOWSORGAN}, "CH", ser=101
#
echo "Create Server Certificate for ${FLOWSERVER}"

cd ${DIR_SERVER}

NAME=${FLOWSERVER}
ORGANISATION="'flow' Working Group"
COUNTRY="CH"
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}/O=${FLOWSORGAN}/C=${FLOWSCOUNT}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="101"
CA=${DIR_CA}/${ISCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
echo "Server Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${README}
openssl verify -CAfile ${SERVER_CA_CHAIN} ${NAME}.crt | tee -a ${README}


#
# Client Certificate for ${USER1}, ser=201
#
echo "Create User Certificare for ${USER1}"

cd ${DIR_CLIENT}

NAME=${USER1}
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="201"
CA=${DIR_CA}/${ICCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
echo "User Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${README}
openssl verify -CAfile ${CLIENT_CA_CHAIN} ${NAME}.crt | tee -a ${README}

openssl rsa -pubout -in ${NAME}.key -out ${NAME}.pub

#
# Client Certificate for ${USER2}, ser=202
#
echo "Create User Certificare for ${USER2}"

NAME=${USER2}
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="202"
CA=${DIR_CA}/${ICCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
echo "User Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${README}
openssl verify -CAfile ${CLIENT_CA_CHAIN} ${NAME}.crt | tee -a ${README}

openssl rsa -pubout -in ${NAME}.key -out ${NAME}.pub
