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
  openssl req -text -noout -in ${FNAME}
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` csrfile1 csrfile2 csrfile3"
  fi
