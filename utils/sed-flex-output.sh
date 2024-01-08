#!/bin/bash

if [ $# -ne 1 ] ; then
  echo "Usage: `basename $0` <input-directory>"
  exit 1
fi

export SED="/usr/bin/sed"
export DIR="${1}"
export REALLY="false"

if [ ! -d ${DIR} ] ; then
  echo "Directory ${DIR} does not exist!"
  exit 1
fi

if [ -f ${DIR}/QasmScanner.yy.cpp ] ; then
  if [ "${REALLY}" = "true" ] ; then
    ${SED} -i 's#yy_flex_debug = 0#yy_flex_debug = 1#g' ${DIR}/QasmScanner.yy.cpp
  else
    echo "Set \${REALLY} above to 'true' if you want Flex debugging."
  fi
fi
