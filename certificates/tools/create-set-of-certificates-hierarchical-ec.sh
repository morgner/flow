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
#     [ v3_ext ]
#     subjectKeyIdentifier   = hash
#     authorityKeyIdentifier = keyid:always,issuer:always
#     basicConstraints       = CA:true
#     [alt_names]
#     DNS.1                  = ${CN}
#
#  The server certificates needat least such file:
#
#     [alt_names]
#     DNS.1 = ${CN}
#
#  named 'ssl.conf' in the execution directory
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

#  secp224r1 : NIST/SECG curve over a 224 bit prime field
#  secp256k1 : SECG curve over a 256 bit prime field
#  secp384r1 : NIST/SECG curve over a 384 bit prime field
#  secp521r1 : NIST/SECG curve over a 521 bit prime field
#  prime256v1: X9.62/SECG curve over a 256 bit prime field

#CURVE=secp224r1
#CURVE=secp256k1
CURVE=secp384r1
#CURVE=secp521r1
#CURVE=prime256v1


CART="cert.flow.info"
SVCA="server-CA"
CLCA="client-CA"

SERVERS="`hostname` `hostname`.localhost.localdomain localhost"
SORGAN="'flow' Working Group (fwg)"
SCNTRY="CH"

CLIENTS="admin agent user recipient sender ${LOGNAME}"
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

openssl ecparam -genkey -name ${CURVE} -out "${DIR_CA}/${NAME}.key"
openssl req     -new -utf8 -sha256 \
                -x509 \
                -days 7305 \
                -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
                -key "${DIR_CA}/${NAME}.key" \
                -out "${DIR_CA}/${NAME}.crt"
echo "CA Root Certificate is: ${DIR_CA}/${NAME}.crt" | tee -a ${HISTORY}


# version 3 extensions for IM-CA
SSL_CONFIG="${WORKDIR}/ssl.conf"
echo "[v3_ext]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always
basicConstraints       = CA:true
" > "${SSL_CONFIG}"

#######################################################################
#
# Server CA
#
echo "Creating Server CA"

NAME="${SVCA}"
CN="Server CA"
ORGANISATION="${SORGAN}"
openssl ecparam -out "${DIR_CA}/${NAME}.key" -name ${CURVE} -genkey
openssl req -new -utf8 -sha256 \
            -days 7301 \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -key "${DIR_CA}/${NAME}.key" \
            -out "${DIR_CA}/${NAME}.csr"
SERIAL="`od -An -N2 -d < /dev/urandom | sed -e 's/\s//g'`"
CA=${CART}
CATO=`openssl x509 -noout -dates -in "${DIR_CA}/${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
openssl x509 -req -days ${DAYS} \
             -extfile "${SSL_CONFIG}" -extensions v3_ext \
             -in "${DIR_CA}/${NAME}.csr" \
             -CA "${DIR_CA}/${CA}.crt" -CAkey "${DIR_CA}/${CA}.key" -set_serial "${SERIAL}" \
             -out "${DIR_CA}/${NAME}.crt"
echo "SERVER:${SERIAL}:Server CA Certificate is: ${DIR_CA}/${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}


#######################################################################
#
#  Client CA
#
echo "Creating Client CA"

NAME=${CLCA}
CN="Client CA"
ORGANISATION="${CORGAN}"
COUNTRY="${CCNTRY}"
openssl ecparam -out "${DIR_CA}/${NAME}.key" -name ${CURVE} -genkey
openssl req -new -utf8 -sha256 \
            -days 7301 \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -key "${DIR_CA}/${NAME}.key" \
            -out "${DIR_CA}/${NAME}.csr"
SERIAL="`od -An -N2 -d < /dev/urandom | sed -e 's/\s//g'`"
CA=${CART}
CATO=`openssl x509 -noout -dates -in "${DIR_CA}/${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
openssl x509 -req -days ${DAYS} \
             -extfile "${SSL_CONFIG}" -extensions v3_ext \
             -in "${DIR_CA}/${NAME}.csr" \
             -CA "${DIR_CA}/${CA}.crt" -CAkey "${DIR_CA}/${CA}.key" -set_serial "${SERIAL}" \
             -out "${DIR_CA}/${NAME}.crt"
echo "CLIENT:${SERIAL}:Client CA Certificate is: ${DIR_CA}/${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}


#######################################################################
#
echo "Create Server Certificates for servers: ${SERVERS}"
#

for C in `egrep "^SERVER:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_SERVER}"
for NAME in ${SERVERS}
  do
  SERIAL=$((SERIAL+1))

echo "[v3_ext]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always
basicConstraints       = CA:false
subjectAltName         = @alt_names
[alt_names]
DNS.1                  = ${NAME}
" > "${SSL_CONFIG}"

  ORGANISATION="${SORGAN}"
  COUNTRY="${SCNTRY}"
# STATE="Bern"
# LOCATION="City of Bern"
  openssl ecparam -out "${NAME}.key" -name ${CURVE} -genkey               
  openssl req -new -utf8 -sha256 \
              -days 7300 \
              -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
              -key "${NAME}.key" \
              -out "${NAME}.csr" 
  CA="${DIR_CA}/${SVCA}"
  CATO=`openssl x509 -noout -dates -in "${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
  DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
  openssl x509 -req \
               -days ${DAYS} \
               -extfile "${SSL_CONFIG}" -extensions v3_ext \
               -in "${NAME}.csr" \
               -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial "${SERIAL}" \
               -out "${NAME}.crt"
  echo "SERVER:${SERIAL}:Certificate for ${NAME} is: `pwd`/${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}
# openssl verify -CAfile ${SERVER_CA_CHAIN} ${NAME}.crt
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
  openssl ecparam -out "${NAME}.key" -name ${CURVE} -genkey               
  openssl req -new -utf8 -sha256 \
              -days 7300 \
              -subj "/CN=${NAME}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
              -key "${NAME}.key" \
              -out "${NAME}.csr"
  CA="${DIR_CA}/${CLCA}"
  CATO=`openssl x509 -noout -dates -in "${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
  DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
  openssl x509 -req \
               -days ${DAYS} \
               -in "${NAME}.csr" \
               -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial "${SERIAL}" \
               -out "${NAME}.crt"
  echo "CLIENT:${SERIAL}:Certificate for ${NAME} is: `pwd`/${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}
  done

rm "${SSL_CONFIG}"

cd "${WORKDIR}"

