#! /usr/bin/env bash

#
#  display-certificates.sh 
#
#  Copyright (C) 2017 by Manfred Morgner
#  manfred@morgner.com
#
#  This script displays the content of certifictes 
#
#  Usage: display-certificates.sh cert1 cert2 cert3
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#

for FNAME in $*
  do
  openssl x509 -text -noout -in ${FNAME}
  done

if [ $# -lt 1 ]
  then
  echo "Usage: `basename $0` cert1 cert2 cert3"
  fi
