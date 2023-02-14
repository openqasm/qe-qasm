#!/bin/bash

if [ $# -ne 3 ] ; then
  echo "Usage: `basename $0` <translation-unit> <license-file> <mode>"
  exit 1
fi

export TU=$1
export LF=$2
export MO=$3

export TUDIR="`dirname ${TU}`"
export TUNAME="`basename ${TU}`"
export UPD="`grep 'IMPORTANT NOTICE' ${TU}`"

if [ "${MO}" != "bison" ] && [ "${MO}" != "flex" ] ; then
  echo "Invalid mode ${MO}."
  exit 1
fi

if [ "${MO}" == "bison" ] ; then
  if [ "${TUNAME}" == "QasmParser.tab.cpp" ] ; then
    if [ -f "${TUDIR}/.${TUNAME}.start" ]; then
      exit 0
    else
      touch "${TUDIR}/.${TUNAME}.start"
      if [ "x${UPD}" == "x" ] ; then
        cat ${LF} > "${TUDIR}/${TUNAME}.$$"
        cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
        cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
        rm -f "${TUDIR}/${TUNAME}.$$"
      fi
      touch "${TUDIR}/.${TUNAME}.done"
      rm -f "${TUDIR}/.${TUNAME}.start"
    fi
  elif [ "${TUNAME}" == "QasmParser.tab.h" ] ; then
    if [ -f "${TUDIR}/.${TUNAME}.start" ]; then
      exit 0
    else
      touch "${TUDIR}/.${TUNAME}.start"
      if [ "x${UPD}" == "x" ] ; then
        cat ${LF} > "${TUDIR}/${TUNAME}.$$"
        cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
        cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
        rm -f "${TUDIR}/${TUNAME}.$$"
      fi
      touch "${TUDIR}/.${TUNAME}.done"
      rm -f "${TUDIR}/.${TUNAME}.start"
    fi
  elif [ "${TUNAME}" == "location.hh" ] ; then
    if [ -f "${TUDIR}/.${TUNAME}.start" ]; then
      exit 0
    else
      touch "${TUDIR}/.${TUNAME}.start"
      if [ "x${UPD}" == "x" ] ; then
        cat ${LF} > "${TUDIR}/${TUNAME}.$$"
        cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
        cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
        rm -f "${TUDIR}/${TUNAME}.$$"
      fi
      touch "${TUDIR}/.${TUNAME}.done"
      rm -f "${TUDIR}/.${TUNAME}.start"
    fi
  fi
elif [ "${MO}" == "flex" ] ; then
  if [ "${TUNAME}" == "QasmScanner.yy.cpp" ] ; then
    if [ -f "${TUDIR}/.${TUNAME}.start" ]; then
      exit 0
    else
      touch "${TUDIR}/.${TUNAME}.start"
      if [ "x${UPD}" == "x" ] ; then
        cat ${LF} > "${TUDIR}/${TUNAME}.$$"
        cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
        cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
        rm -f "${TUDIR}/${TUNAME}.$$"
      fi
      touch "${TUDIR}/.${TUNAME}.done"
      rm -f "${TUDIR}/.${TUNAME}.start"
    fi
  elif [ "${TUNAME}" == "QasmScanner.yy.h" ] ; then
    if [ -f "${TUDIR}/.${TUNAME}.start" ]; then
      exit 0
    else
      touch "${TUDIR}/.${TUNAME}.start"
      if [ "x${UPD}" == "x" ] ; then
        cat ${LF} > "${TUDIR}/${TUNAME}.$$"
        cat ${TU} >> "${TUDIR}/${TUNAME}.$$"
        cp -f "${TUDIR}/${TUNAME}.$$" "${TUDIR}/${TUNAME}"
        rm -f "${TUDIR}/${TUNAME}.$$"
      fi
      touch "${TUDIR}/.${TUNAME}.done"
      rm -f "${TUDIR}/.${TUNAME}.start"
    fi
  fi
fi

