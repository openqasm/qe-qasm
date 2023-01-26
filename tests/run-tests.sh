#!/bin/bash

export HERE="`pwd`"
export RED='\033[0;31m'
export GREEN='\033[0;32m'
export NC='\033[0m'

# Set the following two paths to the ${TOP_SRCDIR}/tests directory:
export TESTDIR="${HERE}/../../qasm/tests/src"
export TESTINC="${HERE}/../../qasm/tests/include"
export TESTLOG="${HERE}/tests.run.out"
cat /dev/null > ${TESTLOG}

# Set this path to the QasmParser example program:
export QASMPARSER="${HERE}/QasmParser"

cd ${TESTDIR}

export TESTLIST="/tmp/qasm-tests.$$"
ls -1 *.qasm > ${TESTLIST}

cd ${HERE}

while read -r file
do
  printf "Testing ${file} ... "
  echo "${file}.out" >> ${TESTLOG} 2>&1
  ${QASMPARSER} -I${TESTINC} ${TESTDIR}/${file} > "${file}.out" 2>&1

  if [ $? -ne 0 ] ; then
    printf "${RED}FAILED!${NC}\n"
  else
    printf "${GREEN}OK.${NC}\n"
  fi
done < ${TESTLIST}

rm -f ${TESTLIST}

