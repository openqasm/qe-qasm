#!/bin/bash

if [ $# -ne 2 ] ; then
  echo "Usage: `basename $0` <translation-unit> <license-file>"
  exit 1
fi

export TU=$1
export LF=$2

export TUDIR="`dirname ${TU}`"
export TUNAME="`basename ${TU}`"

echo "Updating ${TUNAME} ..."

cat ${LF} > "${TUDIR}/${TUNAME}.$$"
cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
rm -f "${TUDIR}/${TUNAME}.$$"
touch "${TUDIR}/${TUNAME}.done"

