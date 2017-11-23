#! /usr/bin/env bash

#
#  create-certificates-selfsigned.sh 
#
#  Copyright (C) 2010 by Manfred Morgner
#  manfred@morgner.com
#
#  This script creates a bunch of certifictes to work with for
#  testing and/or developing (on) the 'flow' system
#
#  Usage: create-certificates-selfsigned.sh name1 name2 name3
#
#  This creates self-signed certificates for name1, name2, name3
#
#     name1.crt (as also name1.key and name1.csr) and so on
#

# DES may be nodes to generate the key unencrypted or des3 to do otherwise
DES=nodes

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


for NAME in $*
  do
# C=CH/ST=SG/L=Wil/O=flow/CN=${NAME}
  SUBJECT="/CN=${NAME}"
  openssl ecparam -out ${NAME}.key -name ${CURVE} -genkey
  openssl req -new -utf8 -sha256 -key ${NAME}.key -${DES} -out ${NAME}.csr -subj "/CN=John Doe"
  openssl x509 -req -days 7300 -in ${NAME}.csr -signkey ${NAME}.key -out ${NAME}.crt
  openssl ec -pubout -in ${NAME}.key -out ${NAME}.pub
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` name1 name2 name3"
  fi
