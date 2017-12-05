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
SVCA="server-CA"

SORGAN="'flow' Working Group (fwg)"
SCNTRY="CH"

WORKDIR=`pwd`
SCRPTDIR=`dirname $0`
HISTORY="${WORKDIR}/CERTIFICATES"

DIR_CA="${WORKDIR}/CA"
DIR_SERVER="${WORKDIR}/server"

SSL_CONFIG="${WORKDIR}/ssl.conf"


#######################################################################
#
echo "Create Server Certificates for servers: ${SERVERS}"
#

for C in `egrep "^SERVER:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done
cd "${DIR_SERVER}"
for NAME in $*
  do
  if [[ -f "${NAME}" && ${NAME##*.} == "csr" ]]
    then
    NAME=${NAME%.*}
    SERIAL=$((SERIAL+1))

echo "[v3_ext]
subjectKeyIdentifier   = hash
authorityKeyIdentifier = keyid:always,issuer:always
basicConstraints       = CA:false
subjectAltName         = @alt_names
[alt_names]
DNS.1                  = ${NAME}
" > "${SSL_CONFIG}"

    CA="${DIR_CA}/${SVCA}"
    CATO=`openssl x509 -noout -dates -in "${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
    DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
    openssl x509 -req -days ${DAYS} \
                 -extfile "${SSL_CONFIG}" -extensions v3_ext \
                 -in "${NAME}.csr" \
                 -CA "${CA}.crt" -CAkey "${CA}.key" \
                 -set_serial ${SERIAL} \
                 -out "${NAME}.crt"
    echo "SERVER:${SERIAL}:Certificate for ${NAME##*/} is: ${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}
    fi
  done

rm "${SSL_CONFIG}"

cd "${WORKDIR}"
