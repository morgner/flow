#! /usr/bin/env bash

#
#  display-csr.sh 
#
#  Copyright (C) 2017 by Manfred Morgner
#  manfred@morgner.com
#
#  This script displays the content of certifictes 
#
#  Usage: display-csr.sh csrfile1 csrfile2 csrfile3
#

for FNAME in $*
  do
  CMD="openssl req -text -noout -in ${FNAME}"
  if [[ -z `which dialog` ]]
    then
    ${CMD}
    else
    dialog --title "${FNAME}" --msgbox "`${CMD}`" 0 0
    clear
    fi
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` csrfile1 csrfile2 csrfile3"
  fi
