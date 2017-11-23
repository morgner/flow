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
CLCA="flow-client-CA"

CORGAN="'flow' User Group (fug)"
CCNTRY="CH"

WORKDIR=`pwd`
SCRPTDIR=`dirname $0`
HISTORY="${WORKDIR}/CERTIFICATES"

DIR_CA="${WORKDIR}/CA"
DIR_CLIENT="${WORKDIR}/client"


#######################################################################
#
echo "Create Client Certificates for clients: ${CLIENTS}"
#

for C in `egrep "^CLIENT:" CERTIFICATES | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_CLIENT}"
for NAME in $*
  do
  SERIAL=$((SERIAL+1))

  CA="${DIR_CA}/${CLCA}"
  openssl x509 -req -days 6840 -in "${NAME}.csr" -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial ${SERIAL} -out "${NAME}.crt"
  echo "CLIENT:${SERIAL}:Certificate for ${NAME} is: `pwd`/${NAME}.crt" | tee -a ${HISTORY}
#  openssl rsa -pubout -in "${NAME}.key" -out "${NAME}.pub"
  openssl verify -CAfile ${CLIENT_CA_CHAIN} ${NAME}.crt
  done
cd "${WORKDIR}"
