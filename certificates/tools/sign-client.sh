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
CLCA="client-CA"

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

for C in `egrep "^CLIENT:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_CLIENT}"
for NAME in $*
  do
  if [[ -f "${NAME}" && ${NAME##*.} == "csr" ]]
    then
    NAME=${NAME%.*}
    SERIAL=$((SERIAL+1))
    CA="${DIR_CA}/${CLCA}"
    CATO=`openssl x509 -noout -dates -in "${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
    DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
    openssl x509 -req -days ${DAYS} -in "${NAME}.csr" -CA "${CA}.crt" -CAkey "${CA}.key" -set_serial ${SERIAL} -out "${NAME}.crt"
    echo "CLIENT:${SERIAL}:Certificate for ${NAME##*/} is: ${NAME}.crt" | tee -a ${HISTORY}
    fi
  done
cd "${WORKDIR}"
