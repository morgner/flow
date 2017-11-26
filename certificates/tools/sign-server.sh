#! /usr/bin/env bash

#  create-set-of-certificates-hierarchical.sh 
#
#  Copyright (C) 2017 by Manfred Morgner
#  manfred@morgner.com
#
#  This script signs server certificates
# 

DES=nodes

CART="cert.flow.info"
SVCA="flow-server-CA"

SORGAN="'flow' Working Group (fwg)"
SCNTRY="CH"

WORKDIR=`pwd`
SCRPTDIR=`dirname $0`
HISTORY="${WORKDIR}/CERTIFICATES"

DIR_CA="${WORKDIR}/CA"
DIR_SERVER="${WORKDIR}/server"


#######################################################################
#
echo "Create Server Certificates for servers: ${SERVERS}"
#

for C in `egrep "^SERVER:" CERTIFICATES | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_SERVER}"
for NAME in $*
  do
  if [[ -f "${NAME}" && ${NAME##*.} == "csr" ]]
    then
    NAME=${NAME%.*}
    SERIAL=$((SERIAL+1))
    CA="${DIR_CA}/${SVCA}"
    openssl x509 -req -days 6840 -in "${NAME}.csr" -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial ${SERIAL} -out "${NAME}.crt"
    echo "SERVER:${SERIAL}:Certificate for ${NAME##*/} is: ${NAME}.crt" | tee -a ${HISTORY}
    fi 
  done
cd "${WORKDIR}"

