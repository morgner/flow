#! /usr/bin/env bash

#  dialog-sign-client-certificate.sh 
#
#  Copyright (C) 2017 by Manfred Morgner
#  manfred.morgner@gmail.com
#
#  This script signs server certificates
#

DIALOG=""

if [ `which dialog` ]; then DIALOG=dialog; TERM=xterm; fi

if [ ! -z ${DISPLAY} ] && [ `which Xdialog` ]
then
	export XDIALOG_HIGH_DIALOG_COMPAT=true
	export XDIALOG_FORCE_AUTOSIZE=true
	DIALOG="Xdialog --fixed-font --left --cr-wrap --no-buttons "
#	DIALOG=dialog
fi

if [ "${DIALOG}" == "" ]
then
	echo "ERROR: No Dialog"
	exit
fi

BT="Sign a Client Certificate Signing Request"

error() {
    ${DIALOG} \
		--backtitle "${BT}" \
		--title "ERROR" --no-collapse --msgbox "${1}" 0 0
}


if [[ $# -eq 0 ]]
then
	while [ "${NAME}" = "" ]
	do

	exec 3>&1
	NAME=`${DIALOG} \
			--backtitle "${BT}" \
			--title "Choose one CSR" \
			--clear \
			--cancel-label "Abort" \
			--fselect '' 0 0 2>&1 1>&3`
	DLGRES=$?
#	echo "R:${DLGRES}"
	exec 3>&-
	if [[ ${DLGRES} -ne 0 ]]
	then
#		echo "Abort"
		exit
	fi

	done

else
	NAME=$1
fi

if [[ "${NAME}" > "" ]]
then
	echo "CSR=${NAME}"
else
#	this point can't be reached anymore?
	error "No file CSR given"
fi

openssl req -noout -in ${NAME} 2>/dev/null
if [[ $? -ne 0 ]]
then
	error "${NAME} is not a valid Signing Request"
	exit
fi


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
echo "Create Client Certificate out of ${NAME}"
#

for C in `egrep "^CLIENT:" ${HISTORY} | cut -d: -f2`; do SERIAL=${C}; done

if [[ -f "${NAME}" && ${NAME##*.} == "csr" ]]
then
    NAME="${NAME%.*}"
    SERIAL=$((SERIAL+1))
    CA="${DIR_CA}/${CLCA}"
    CATO=`openssl x509 -noout -dates -in "${CA}.crt" | tail -1 | sed -e "s/^.*=\(.*\) ..:..:.. \(....\).*$/\1 \2/"`
    DAYS="$((($(date -d "${CATO}" '+%s') - $(date '+%s'))/(24*3600)-1))"
    openssl x509 -req -days ${DAYS} \
	             -in "${NAME}.csr" \
	             -CA "${CA}.crt" -CAkey "${CA}.key" \
	             -set_serial ${SERIAL} \
	             -out "${NAME}.crt"
    echo "CLIENT:${SERIAL}:Certificate for ${NAME##*/} is: ${NAME}.crt (${DAYS} days)" | tee -a ${HISTORY}
fi

cd "${WORKDIR}"

