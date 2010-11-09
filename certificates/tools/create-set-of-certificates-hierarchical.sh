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
#

RTCA="www.überfluß.de"
ISCA="Intermediate-Server-CA"
ICCA="Intermediate-Client-CA"

FLOWSERVER="www.example.ch"
FLOWSORGAN="'flow' Working Group"
FLOWSCOUNT="CH"

USER1="username"
USER2="recipient"


#
# Root CA
#
NAME=${RTCA}
CN=${NAME}
ORGANISATION="Great Privacy Commitee"
COUNTRY="DE"
STATE="Thüringen"
LOCATION="Weimar"
openssl req -new -utf8 -x509 -days 7302 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.crt

#
# Intermediate Server CA
#
NAME=${ISCA}
CN="Intermediate Server CA"
ORGANISATION="Great Privacy Server Management"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="100"
CA=${RTCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt

#
# Intermediate Client CA
#
NAME=${ICCA}
CN="Intermediate Client CA"
ORGANISATION="Great Privacy Client Administration"
openssl req -new -utf8 -days 7301 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${CN}/O=${ORGANISATION}/C=${COUNTRY}/ST=${STATE}/L=${LOCATION}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="200"
CA=${RTCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt


#
# Server Certificate for ${FLOWSERVER}, ${FLOWSORGAN}, "CH", ser=101
#

NAME=${FLOWSERVER}
ORGANISATION="'flow' Working Group"
COUNTRY="CH"
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}/O=${FLOWSORGAN}/C=${FLOWSCOUNT}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="101"
CA=${ISCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt


#
# Client Certificate for ${USER1}, ser=201
#

NAME=${USER1}
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="201"
CA=${ICCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt


#
# Client Certificate for ${USER2}, ser=202
#

NAME=${USER2}
openssl req -new -utf8 -days 7300 \
            -newkey rsa:4096 -nodes \
            -subj "/CN=${NAME}" \
            -keyout ${NAME}.key -out ${NAME}.csr
SER="202"
CA=${ICCA}
openssl x509 -req -days 7300 -in ${NAME}.csr -CA ${CA}.crt -CAkey ${CA}.key -set_serial ${SER} -out ${NAME}.crt
