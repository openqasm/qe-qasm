#!/bin/bash

export HERE="`pwd`"
export TESTFILE="${HERE}/tests.run.out"

NT="`cat ${TESTFILE} | wc -l`"
echo "There are ${NT} tests."

while read -r file
do
  echo "--------------------------------------"
  TESTNAME="`echo ${file} | sed 's#.out##g'`"
  echo "Test Program: ${TESTNAME}"
  echo "Test Output : ${file}"
  echo -n "Status: "
  egrep '^OK.|fail|Fail|error|Error' ${file}
  echo "--------------------------------------"
done < ${TESTFILE}

