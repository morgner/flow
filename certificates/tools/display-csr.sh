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
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#

for FNAME in $*
  do
  openssl req -text -noout -in ${FNAME}
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` csrfile1 csrfile2 csrfile3"
  fi
