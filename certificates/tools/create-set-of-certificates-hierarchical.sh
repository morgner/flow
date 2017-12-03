#! /usr/bin/env bash

#  create-set-of-certificates-hierarchical.sh 
#
#  Copyright (C) 2010 by Manfred Morgner
#  manfred@morgner.com
#
#  This script creates a bunch of certifictes to work with for 'flow'
# 
#  The intermediate CAs require a file with the following content:
# 
#     [ v3_ca ]
#     subjectKeyIdentifier   = hash
#     authorityKeyIdentifier = keyid:always,issuer:always
#     basicConstraints       = CA:true
# 
#  named 'v3_ca.conf' in the execution directory
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

DES=nodes

CART="cert.flow.info"
SVCA="flow-server-CA"
CLCA="flow-client-CA"

SERVERS="`hostname` localhost"
SORGAN="'flow' Working Group (fwg)"
SCNTRY="CH"

CLIENTS="username recipient ${LOGNAME}"
CORGAN="'flow' User Group (fug)"
CCNTRY="CH"

WORKDIR=`pwd`
SCRPTDIR=`dirname $0`
HISTORY="${WORKDIR}/CERTIFICATES"
rm -f ${HISTORY}

mkdir -p CA
mkdir -p client
mkdir -p server

DIR_CA="${WORKDIR}/CA"
DIR_SERVER="${WORKDIR}/server"
DIR_CLIENT="${WORKDIR}/client"

SERVER_CA_CHAIN="${DIR_SERVER}/server-ca-chain.pem"
CLIENT_CA_CHAIN="${DIR_CLIENT}/client-ca-chain.pem"


#######################################################################
#
# Root CA
#
echo "Creating CA Root"

cd ${DIR_CA}

NAME="${CART}"
ORGANISATION="${SORGAN}"
COUNTRY="${SCNTRY}"
STATE="Bern"
LOCATION="City of Bern"
openssl req -new -utf8 -x509 -days 7302 \
            -newkey rsa:4096 -${DES} \
            -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout "${DIR_CA}/${NAME}.key" -out "${DIR_CA}/${NAME}.crt"
echo "CA Root Certificate is: ${DIR_CA}/${NAME}.crt" | tee -a ${HISTORY}


# version 3 extensions for IM-CA
V3_CA_FILENAME="${WORKDIR}/v3_ca.conf"
echo "[ v3_ca ]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always
basicConstraints       = CA:true
" > "${V3_CA_FILENAME}"

#######################################################################
#
# Server CA
#
echo "Creating Server CA"

NAME="${SVCA}"
CN="Server CA"
ORGANISATION="${SORGAN}"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -${DES} \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout "${DIR_CA}/${NAME}.key" -out "${DIR_CA}/${NAME}".csr
SERIAL="100"
CA=${CART}
openssl x509 -req -days 7301 \
             -extfile "${V3_CA_FILENAME}" -extensions v3_ca \
             -in "${DIR_CA}/${NAME}".csr \
             -CA "${DIR_CA}/${CA}.crt" -CAkey "${DIR_CA}/${CA}.key" -set_serial ${SERIAL} \
             -out "${DIR_CA}/${NAME}.crt"
echo "SERVER:${SERIAL}:CA Certificate is: ${DIR_CA}/${NAME}.crt" | tee -a ${HISTORY}


#######################################################################
#
#  Client CA
#
echo "Creating Client CA"

NAME=${CLCA}
CN="Client CA"
ORGANISATION="${CORGAN}"
COUNTRY="${CCNTRY}"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -${DES} \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout "${DIR_CA}/${NAME}.key" -out "${DIR_CA}/${NAME}.csr"
SERIAL="200"
CA=${CART}
openssl x509 -req -days 7301 \
             -extfile "${V3_CA_FILENAME}" -extensions v3_ca \
             -in "${DIR_CA}/${NAME}.csr" \
             -CA "${DIR_CA}/${CA}.crt" -CAkey "${DIR_CA}/${CA}.key" -set_serial ${SERIAL} \
             -out "${DIR_CA}/${NAME}.crt"
echo "CLIENT:${SERIAL}:Client CA Certificate is: ${DIR_CA}/${NAME}.crt" | tee -a ${HISTORY}

# version 3 extensions for IM-CA
rm "${V3_CA_FILENAME}"


#######################################################################
#
# Compose the CA Chains
#
echo "Compose CA Chains"

rm -f ${SERVER_CA_CHAIN}
for i in ${CART}.crt ${SVCA}.crt
  do
  openssl x509 -in "${DIR_CA}/$i" -text >> "${SERVER_CA_CHAIN}"
  done
echo "Server CA Chain is: ${SERVER_CA_CHAIN}" | tee -a ${HISTORY}

rm -f ${CLIENT_CA_CHAIN}
for i in ${CART}.crt ${CLCA}.crt
  do
  openssl x509 -in "${DIR_CA}/$i" -text >> "${CLIENT_CA_CHAIN}"
  done
echo "Client CA Chain is: ${CLIENT_CA_CHAIN}" | tee -a ${HISTORY}


#######################################################################
#
echo "Create Server Certificates for servers: ${SERVERS}"
#

for C in `egrep "^SERVER:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_SERVER}"
for NAME in ${SERVERS}
  do
  SERIAL=$((SERIAL+1))

  ORGANISATION="${SORGAN}"
  COUNTRY="${SCNTRY}"
# STATE="Bern"
# LOCATION="City of Bern"
  openssl req  -new -utf8 -days 7300 \
               -newkey rsa:4096 -${DES} \
               -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
               -keyout "${NAME}.key" -out ${NAME}.csr
  CA="${DIR_CA}/${SVCA}"
  openssl x509 -req -days 7300 -in "${NAME}.csr" -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial ${SERIAL} -out "${NAME}.crt"
  echo "SERVER:${SERIAL}:Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${HISTORY}
  openssl verify -CAfile ${SERVER_CA_CHAIN} ${NAME}.crt
  done
cd "${WORKDIR}"


#######################################################################
#
echo "Create Client Certificates for clients: ${CLIENTS}"
#

for C in `egrep "^CLIENT:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done
cd ${DIR_CLIENT}
for NAME in ${CLIENTS}
  do
  SERIAL=$((SERIAL+1))

  ORGANISATION="${CORGAN}"
  COUNTRY="${CCNTRY}"
# STATE="Bern"
# LOCATION="City of Bern"
  openssl req -new -utf8 -days 7300 \
              -newkey rsa:4096 -${DES} \
              -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
              -keyout "${NAME}.key" -out "${NAME}.csr"
  CA="${DIR_CA}/${CLCA}"
  openssl x509 -req -days 7300 -in "${NAME}.csr" -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial ${SERIAL} -out "${NAME}.crt"
  echo "CLIENT:${SERIAL}:Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${HISTORY}
  openssl rsa -pubout -in "${NAME}.key" -out "${NAME}.pub"
  openssl verify -CAfile ${CLIENT_CA_CHAIN} ${NAME}.crt
  done
cd "${WORKDIR}"
