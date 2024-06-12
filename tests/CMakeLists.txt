enable_testing()
include (CTest)
include(FindUnixCommands)

set(OPENQASM_TEST_SRCDIR "${CMAKE_SOURCE_DIR}/tests/src")
set(OPENQASM_TEST_INCDIR "${CMAKE_SOURCE_DIR}/tests/include")
set(OPENQASM_TEST_PROGRAM "${CMAKE_BINARY_DIR}/bin/QasmParser")

add_test(NAME t00000
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/adder_8.qasm > ${CMAKE_BINARY_DIR}/tests/adder_8.qasm.out 2>&1")
add_test(NAME t00001
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/barrier.qasm > ${CMAKE_BINARY_DIR}/tests/barrier.qasm.out 2>&1")
add_test(NAME t00002
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/cphase.qasm > ${CMAKE_BINARY_DIR}/tests/cphase.qasm.out 2>&1")
add_test(NAME t00003
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/newlineifelse.qasm > ${CMAKE_BINARY_DIR}/tests/newlineifelse.qasm.out 2>&1")
add_test(NAME t00004
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/nospace-ifelse.qasm > ${CMAKE_BINARY_DIR}/tests/nospace-ifelse.qasm.out 2>&1")
add_test(NAME t00005
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qft1.qasm > ${CMAKE_BINARY_DIR}/tests/qft1.qasm.out 2>&1")
add_test(NAME t00006
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qft2.qasm > ${CMAKE_BINARY_DIR}/tests/qft2.qasm.out 2>&1")
add_test(NAME t00007
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qft4.qasm > ${CMAKE_BINARY_DIR}/tests/qft4.qasm.out 2>&1")
add_test(NAME t00008
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/tof_10.qasm > ${CMAKE_BINARY_DIR}/tests/tof_10.qasm.out 2>&1")
add_test(NAME t00009
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/tof_3.qasm > ${CMAKE_BINARY_DIR}/tests/tof_3.qasm.out 2>&1")
add_test(NAME t00010
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/tof_4.qasm > ${CMAKE_BINARY_DIR}/tests/tof_4.qasm.out 2>&1")
add_test(NAME t00011
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/tof_5.qasm > ${CMAKE_BINARY_DIR}/tests/tof_5.qasm.out 2>&1")
add_test(NAME t00012
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qft1.qasm > ${CMAKE_BINARY_DIR}/tests/qft1.qasm.out 2>&1")
add_test(NAME t00013
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qft2.qasm > ${CMAKE_BINARY_DIR}/tests/qft2.qasm.out 2>&1")
add_test(NAME t00014
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/teleportation-1.qasm > ${CMAKE_BINARY_DIR}/tests/teleportation-1.qasm.out 2>&1")
add_test(NAME t00015
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/teleportation-2.qasm > ${CMAKE_BINARY_DIR}/tests/teleportation-2.qasm.out 2>&1")
add_test(NAME t00016
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/teleportation-3.qasm > ${CMAKE_BINARY_DIR}/tests/teleportation-3.qasm.out 2>&1")
add_test(NAME t00017
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-qubit.qasm > ${CMAKE_BINARY_DIR}/tests/test-qubit.qasm.out 2>&1")
add_test(NAME t00018
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-const.qasm > ${CMAKE_BINARY_DIR}/tests/test-const.qasm.out 2>&1")
add_test(NAME t00019
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/hadamard.qasm > ${CMAKE_BINARY_DIR}/tests/hadamard.qasm.out 2>&1")
add_test(NAME t00020
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/angle-theta.qasm > ${CMAKE_BINARY_DIR}/tests/angle-theta.qasm.out 2>&1")
add_test(NAME t00021
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-angle.qasm > ${CMAKE_BINARY_DIR}/tests/test-angle.qasm.out 2>&1")
add_test(NAME t00022
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-angle-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-angle-cast.qasm.out 2>&1")
add_test(NAME t00023
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-big-int.qasm > ${CMAKE_BINARY_DIR}/tests/test-big-int.qasm.out 2>&1")
add_test(NAME t00024
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bitset-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-bitset-cast.qasm.out 2>&1")
add_test(NAME t00025
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bool.qasm > ${CMAKE_BINARY_DIR}/tests/test-bool.qasm.out 2>&1")
add_test(NAME t00026
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bool-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-bool-2.qasm.out 2>&1")
add_test(NAME t00027
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bool-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-bool-3.qasm.out 2>&1")
add_test(NAME t00028
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-1.qasm.out 2>&1")
add_test(NAME t00029
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-2.qasm.out 2>&1")
add_test(NAME t00030
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-3.qasm.out 2>&1")
add_test(NAME t00031
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-4.qasm.out 2>&1")
add_test(NAME t00032
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-switch.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-switch.qasm.out 2>&1")
add_test(NAME t00033
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-1.qasm.out 2>&1")
add_test(NAME t00034
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-2.qasm.out 2>&1")
add_test(NAME t00035
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-3.qasm.out 2>&1")
add_test(NAME t00036
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-4.qasm.out 2>&1")
add_test(NAME t00037
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-5.qasm.out 2>&1")
add_test(NAME t00038
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-6.qasm.out 2>&1")
add_test(NAME t00039
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-7.qasm.out 2>&1")
add_test(NAME t00040
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-const.qasm > ${CMAKE_BINARY_DIR}/tests/test-const.qasm.out 2>&1")
add_test(NAME t00041
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-qubit.qasm > ${CMAKE_BINARY_DIR}/tests/test-qubit.qasm.out 2>&1")
add_test(NAME t00042
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-euler.qasm > ${CMAKE_BINARY_DIR}/tests/test-euler.qasm.out 2>&1")
add_test(NAME t00043
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pi.qasm > ${CMAKE_BINARY_DIR}/tests/test-pi.qasm.out 2>&1")
add_test(NAME t00044
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-tau.qasm > ${CMAKE_BINARY_DIR}/tests/test-tau.qasm.out 2>&1")
add_test(NAME t00045
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-1.qasm.out 2>&1")
add_test(NAME t00046
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-2.qasm.out 2>&1")
add_test(NAME t00047
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-3.qasm.out 2>&1")
add_test(NAME t00048
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-4.qasm.out 2>&1")
add_test(NAME t00049
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-5.qasm.out 2>&1")
add_test(NAME t00050
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-6.qasm.out 2>&1")
add_test(NAME t00051
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-7.qasm.out 2>&1")
add_test(NAME t00052
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-8.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-8.qasm.out 2>&1")
add_test(NAME t00053
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-9.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-9.qasm.out 2>&1")
add_test(NAME t00054
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-10.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-10.qasm.out 2>&1")
add_test(NAME t00055
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-11.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-11.qasm.out 2>&1")
add_test(NAME t00056
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-12.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-12.qasm.out 2>&1")
add_test(NAME t00057
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ifelse-13.qasm > ${CMAKE_BINARY_DIR}/tests/test-ifelse-13.qasm.out 2>&1")
add_test(NAME t00058
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-shift-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-shift-1.qasm.out 2>&1")
add_test(NAME t00059
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-shift-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-shift-2.qasm.out 2>&1")
add_test(NAME t00060
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-shift-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-shift-3.qasm.out 2>&1")
add_test(NAME t00061
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-1.qasm.out 2>&1")
add_test(NAME t00062
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-2.qasm.out 2>&1")
add_test(NAME t00063
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-3.qasm.out 2>&1")
add_test(NAME t00064
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-4.qasm.out 2>&1")
add_test(NAME t00065
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-5.qasm.out 2>&1")
add_test(NAME t00066
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-6.qasm.out 2>&1")
add_test(NAME t00067
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-for-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-for-7.qasm.out 2>&1")
add_test(NAME t00068
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-induction-var-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-induction-var-1.qasm.out 2>&1")
add_test(NAME t00069
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-induction-var-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-induction-var-2.qasm.out 2>&1")
add_test(NAME t00070
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-induction-var-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-induction-var-3.qasm.out 2>&1")
add_test(NAME t00071
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-while-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-while-1.qasm.out 2>&1")
add_test(NAME t00072
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-while-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-while-2.qasm.out 2>&1")
add_test(NAME t00073
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-while-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-while-3.qasm.out 2>&1")
add_test(NAME t00074
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-assign.qasm > ${CMAKE_BINARY_DIR}/tests/test-assign.qasm.out 2>&1")
add_test(NAME t00075
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cast-bool.qasm > ${CMAKE_BINARY_DIR}/tests/test-cast-bool.qasm.out 2>&1")
add_test(NAME t00076
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-no-openqasm-line-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-no-openqasm-line-1.qasm.out 2>&1")
add_test(NAME t00077
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-no-openqasm-line-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-no-openqasm-line-2.qasm.out 2>&1")
add_test(NAME t00078
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-no-openqasm-line-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-no-openqasm-line-3.qasm.out 2>&1")
add_test(NAME t00079
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-c-comment.qasm > ${CMAKE_BINARY_DIR}/tests/test-c-comment.qasm.out 2>&1")
add_test(NAME t00080
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-consecutive-gates-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-consecutive-gates-1.qasm.out 2>&1")
add_test(NAME t00081
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-consecutive-gates-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-consecutive-gates-2.qasm.out 2>&1")
add_test(NAME t00082
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-add-assign.qasm > ${CMAKE_BINARY_DIR}/tests/test-add-assign.qasm.out 2>&1")
add_test(NAME t00083
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-sub.qasm > ${CMAKE_BINARY_DIR}/tests/test-sub.qasm.out 2>&1")
add_test(NAME t00084
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-eq.qasm > ${CMAKE_BINARY_DIR}/tests/test-eq.qasm.out 2>&1")
add_test(NAME t00085
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-operators.qasm > ${CMAKE_BINARY_DIR}/tests/test-operators.qasm.out 2>&1")
add_test(NAME t00086
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-operators-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-operators-2.qasm.out 2>&1")
add_test(NAME t00087
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-mpdecimal.qasm > ${CMAKE_BINARY_DIR}/tests/test-mpdecimal.qasm.out 2>&1")
add_test(NAME t00088
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-unary-ops.qasm > ${CMAKE_BINARY_DIR}/tests/test-unary-ops.qasm.out 2>&1")
add_test(NAME t00089
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-1.qasm.out 2>&1")
add_test(NAME t00090
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-2.qasm.out 2>&1")
add_test(NAME t00091
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-3.qasm.out 2>&1")
add_test(NAME t00092
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-4.qasm.out 2>&1")
add_test(NAME t00093
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-5.qasm.out 2>&1")
add_test(NAME t00094
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-6.qasm.out 2>&1")
add_test(NAME t00095
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-7.qasm.out 2>&1")
add_test(NAME t00096
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-8.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-8.qasm.out 2>&1")
add_test(NAME t00097
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-9.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-9.qasm.out 2>&1")
add_test(NAME t00098
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-10.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-10.qasm.out 2>&1")
add_test(NAME t00099
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-xor-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-xor-1.qasm.out 2>&1")
add_test(NAME t00100
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cx.qasm > ${CMAKE_BINARY_DIR}/tests/test-cx.qasm.out 2>&1")
add_test(NAME t00101
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cos.qasm > ${CMAKE_BINARY_DIR}/tests/test-cos.qasm.out 2>&1")
add_test(NAME t00102
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-phase.qasm > ${CMAKE_BINARY_DIR}/tests/test-phase.qasm.out 2>&1")
add_test(NAME t00103
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-hadamard.qasm > ${CMAKE_BINARY_DIR}/tests/test-hadamard.qasm.out 2>&1")
add_test(NAME t00104
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-scalars.qasm > ${CMAKE_BINARY_DIR}/tests/test-scalars.qasm.out 2>&1")
add_test(NAME t00105
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-add-assign.qasm > ${CMAKE_BINARY_DIR}/tests/test-add-assign.qasm.out 2>&1")
add_test(NAME t00106
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-if.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-if.qasm.out 2>&1")
add_test(NAME t00107
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-redeclarations.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-redeclarations.qasm.out 2>&1")
add_test(NAME t00108
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return.qasm > ${CMAKE_BINARY_DIR}/tests/test-return.qasm.out 2>&1")
add_test(NAME t00109
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-2.qasm.out 2>&1")
add_test(NAME t00110
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-3.qasm.out 2>&1")
add_test(NAME t00111
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-xor.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-xor.qasm.out 2>&1")
add_test(NAME t00112
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-kernel-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-kernel-1.qasm.out 2>&1")
add_test(NAME t00113
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-kernel-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-kernel-2.qasm.out 2>&1")
add_test(NAME t00114
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-kernel-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-kernel-3.qasm.out 2>&1")
add_test(NAME t00115
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-kernel-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-kernel-4.qasm.out 2>&1")
add_test(NAME t00116
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal.qasm.out 2>&1")
add_test(NAME t00117
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-reset.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-reset.qasm.out 2>&1")
add_test(NAME t00118
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-measure.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-measure.qasm.out 2>&1")
add_test(NAME t00119
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-binop.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-binop.qasm.out 2>&1")
add_test(NAME t00120
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-conversion.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-conversion.qasm.out 2>&1")
add_test(NAME t00121
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-unop.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-unop.qasm.out 2>&1")
add_test(NAME t00122
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-measure.qasm > ${CMAKE_BINARY_DIR}/tests/test-measure.qasm.out 2>&1")
add_test(NAME t00123
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-measure-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-measure-2.qasm.out 2>&1")
add_test(NAME t00124
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-void-measure.qasm > ${CMAKE_BINARY_DIR}/tests/test-void-measure.qasm.out 2>&1")
add_test(NAME t00125
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-opaque.qasm > ${CMAKE_BINARY_DIR}/tests/test-opaque.qasm.out 2>&1")
add_test(NAME t00126
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-1.qasm.out 2>&1")
add_test(NAME t00127
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-2.qasm.out 2>&1")
add_test(NAME t00128
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-3.qasm.out 2>&1")
add_test(NAME t00129
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-4.qasm.out 2>&1")
add_test(NAME t00130
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-5.qasm.out 2>&1")
add_test(NAME t00131
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-let-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-let-6.qasm.out 2>&1")
add_test(NAME t00132
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-fixed-scalars.qasm > ${CMAKE_BINARY_DIR}/tests/test-fixed-scalars.qasm.out 2>&1")
add_test(NAME t00133
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ctrl.qasm > ${CMAKE_BINARY_DIR}/tests/test-ctrl.qasm.out 2>&1")
add_test(NAME t00134
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-creg.qasm > ${CMAKE_BINARY_DIR}/tests/test-creg.qasm.out 2>&1")
add_test(NAME t00135
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-qreg-v2.qasm > ${CMAKE_BINARY_DIR}/tests/test-qreg-v2.qasm.out 2>&1")
add_test(NAME t00136
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-qreg-v3.qasm > ${CMAKE_BINARY_DIR}/tests/test-qreg-v3.qasm.out 2>&1")
add_test(NAME t00137
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-1.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-1.qasm.out 2>&1")
add_test(NAME t00138
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-2-3.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-2-3.qasm.out 2>&1")
add_test(NAME t00139
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-4.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-4.qasm.out 2>&1")
add_test(NAME t00140
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-5.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-5.qasm.out 2>&1")
add_test(NAME t00141
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-6.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-6.qasm.out 2>&1")
add_test(NAME t00142
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-7.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-7.qasm.out 2>&1")
add_test(NAME t00143
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-8.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-8.qasm.out 2>&1")
add_test(NAME t00144
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-9.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-9.qasm.out 2>&1")
add_test(NAME t00145
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/iqpe-10.qasm > ${CMAKE_BINARY_DIR}/tests/iqpe-10.qasm.out 2>&1")
add_test(NAME t00146
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-box-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-box-1.qasm.out 2>&1")
add_test(NAME t00147
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-box-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-box-2.qasm.out 2>&1")
add_test(NAME t00148
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-box-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-box-3.qasm.out 2>&1")
add_test(NAME t00149
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-box-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-box-4.qasm.out 2>&1")
add_test(NAME t00150
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-opaque.qasm > ${CMAKE_BINARY_DIR}/tests/test-opaque.qasm.out 2>&1")
add_test(NAME t00151
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-delay.qasm > ${CMAKE_BINARY_DIR}/tests/test-delay.qasm.out 2>&1")
add_test(NAME t00152
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-delay-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-delay-2.qasm.out 2>&1")
add_test(NAME t00153
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-1.qasm.out 2>&1")
add_test(NAME t00154
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-2.qasm.out 2>&1")
add_test(NAME t00155
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-3.qasm.out 2>&1")
add_test(NAME t00156
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-4.qasm.out 2>&1")
add_test(NAME t00157
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-5.qasm.out 2>&1")
add_test(NAME t00158
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-6.qasm.out 2>&1")
add_test(NAME t00159
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-7.qasm.out 2>&1")
add_test(NAME t00160
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-8.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-8.qasm.out 2>&1")
add_test(NAME t00161
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-9.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-9.qasm.out 2>&1")
add_test(NAME t00162
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-func-call-10.qasm > ${CMAKE_BINARY_DIR}/tests/test-func-call-10.qasm.out 2>&1")
add_test(NAME t00163
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-sign-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-sign-1.qasm.out 2>&1")
add_test(NAME t00164
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-sign-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-sign-2.qasm.out 2>&1")
add_test(NAME t00165
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/bell_v0.qasm > ${CMAKE_BINARY_DIR}/tests/bell_v0.qasm.out 2>&1")
add_test(NAME t00166
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/bell_v1.qasm > ${CMAKE_BINARY_DIR}/tests/bell_v1.qasm.out 2>&1")
add_test(NAME t00167
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/reset_v0.qasm > ${CMAKE_BINARY_DIR}/tests/reset_v0.qasm.out 2>&1")
add_test(NAME t00168
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/reset_v1.qasm > ${CMAKE_BINARY_DIR}/tests/reset_v1.qasm.out 2>&1")
add_test(NAME t00169
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/qss-teleport.qasm > ${CMAKE_BINARY_DIR}/tests/qss-teleport.qasm.out 2>&1")
add_test(NAME t00170
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-1.qasm.out 2>&1")
add_test(NAME t00171
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-2.qasm.out 2>&1")
add_test(NAME t00172
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-3.qasm.out 2>&1")
add_test(NAME t00173
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-4.qasm.out 2>&1")
add_test(NAME t00174
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-5.qasm.out 2>&1")
add_test(NAME t00175
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-6.qasm.out 2>&1")
add_test(NAME t00176
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-7.qasm.out 2>&1")
add_test(NAME t00177
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-8.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-8.qasm.out 2>&1")
add_test(NAME t00178
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-9.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-9.qasm.out 2>&1")
add_test(NAME t00179
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-10.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-10.qasm.out 2>&1")
add_test(NAME t00180
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-11.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-11.qasm.out 2>&1")
add_test(NAME t00181
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-12.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-12.qasm.out 2>&1")
add_test(NAME t00182
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-13.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-13.qasm.out 2>&1")
add_test(NAME t00183
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-14.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-14.qasm.out 2>&1")
add_test(NAME t00184
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-15.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-15.qasm.out 2>&1")
add_test(NAME t00185
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-array-16.qasm > ${CMAKE_BINARY_DIR}/tests/test-array-16.qasm.out 2>&1")
add_test(NAME t00186
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-extern-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-extern-1.qasm.out 2>&1")
add_test(NAME t00187
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-extern-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-extern-2.qasm.out 2>&1")
add_test(NAME t00188
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-extern-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-extern-3.qasm.out 2>&1")
add_test(NAME t00189
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-extern-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-extern-4.qasm.out 2>&1")
add_test(NAME t00190
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-duration-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-duration-1.qasm.out 2>&1")
add_test(NAME t00191
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-duration-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-duration-2.qasm.out 2>&1")
add_test(NAME t00192
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-1.qasm.out 2>&1")
add_test(NAME t00193
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-2.qasm.out 2>&1")
add_test(NAME t00194
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-3.qasm.out 2>&1")
add_test(NAME t00195
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-4.qasm.out 2>&1")
add_test(NAME t00196
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-5.qasm.out 2>&1")
add_test(NAME t00197
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-6.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-6.qasm.out 2>&1")
add_test(NAME t00198
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-7.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-7.qasm.out 2>&1")
add_test(NAME t00199
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-8.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-8.qasm.out 2>&1")
add_test(NAME t00200
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-9.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-9.qasm.out 2>&1")
add_test(NAME t00201
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-10.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-10.qasm.out 2>&1")
add_test(NAME t00202
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-11.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-11.qasm.out 2>&1")
add_test(NAME t00203
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-complex-12.qasm > ${CMAKE_BINARY_DIR}/tests/test-complex-12.qasm.out 2>&1")
add_test(NAME t00204
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ugate-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-ugate-1.qasm.out 2>&1")
add_test(NAME t00205
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ugate-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-ugate-2.qasm.out 2>&1")
add_test(NAME t00206
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ugate-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-ugate-3.qasm.out 2>&1")
add_test(NAME t00207
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-inverse-trig-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-inverse-trig-1.qasm.out 2>&1")
add_test(NAME t00208
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-inverse-trig-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-inverse-trig-2.qasm.out 2>&1")
add_test(NAME t00209
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-function-gate-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-function-gate-1.qasm.out 2>&1")
add_test(NAME t00210
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-function-param-names.qasm > ${CMAKE_BINARY_DIR}/tests/test-function-param-names.qasm.out 2>&1")
add_test(NAME t00211
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-negative-expression.qasm > ${CMAKE_BINARY_DIR}/tests/test-negative-expression.qasm.out 2>&1")
add_test(NAME t00212
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-negative-angle.qasm > ${CMAKE_BINARY_DIR}/tests/test-negative-angle.qasm.out 2>&1")
add_test(NAME t00213
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-loop.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-loop.qasm.out 2>&1")
add_test(NAME t00214
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-unary-pi.qasm > ${CMAKE_BINARY_DIR}/tests/test-unary-pi.qasm.out 2>&1")
add_test(NAME t00215
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-unicode.qasm > ${CMAKE_BINARY_DIR}/tests/test-unicode.qasm.out 2>&1")
add_test(NAME t00216
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-chinese-simplified-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-chinese-simplified-utf8.qasm.out 2>&1")
add_test(NAME t00217
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-chinese-traditional-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-chinese-traditional-utf8.qasm.out 2>&1")
add_test(NAME t00218
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-german-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-german-utf8.qasm.out 2>&1")
add_test(NAME t00219
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-hiragana-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-hiragana-utf8.qasm.out 2>&1")
add_test(NAME t00220
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-katakana-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-katakana-utf8.qasm.out 2>&1")
add_test(NAME t00221
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-korean-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-korean-utf8.qasm.out 2>&1")
add_test(NAME t00222
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-russian-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-russian-utf8.qasm.out 2>&1")
add_test(NAME t00223
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-swedish-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-swedish-utf8.qasm.out 2>&1")
add_test(NAME t00224
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-umlaut-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-umlaut-utf8.qasm.out 2>&1")
add_test(NAME t00225
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-underscore-1-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-underscore-1-utf8.qasm.out 2>&1")
add_test(NAME t00226
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-underscore-2-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-underscore-2-utf8.qasm.out 2>&1")
add_test(NAME t00227
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-underscore-3-utf8.qasm > ${CMAKE_BINARY_DIR}/tests/test-underscore-3-utf8.qasm.out 2>&1")
add_test(NAME t00228
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-popcount.qasm > ${CMAKE_BINARY_DIR}/tests/test-popcount.qasm.out 2>&1")
add_test(NAME t00229
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-popcount-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-popcount-2.qasm.out 2>&1")
add_test(NAME t00230
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-rotl.qasm > ${CMAKE_BINARY_DIR}/tests/test-rotl.qasm.out 2>&1")
add_test(NAME t00231
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-rotr.qasm > ${CMAKE_BINARY_DIR}/tests/test-rotr.qasm.out 2>&1")
add_test(NAME t00232
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bitset-bitops.qasm > ${CMAKE_BINARY_DIR}/tests/test-bitset-bitops.qasm.out 2>&1")
add_test(NAME t00233
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-control-chain-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-control-chain-1.qasm.out 2>&1")
add_test(NAME t00234
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-control-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-control-1.qasm.out 2>&1")
add_test(NAME t00235
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-control-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-control-2.qasm.out 2>&1")
add_test(NAME t00236
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-gphase.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-gphase.qasm.out 2>&1")
add_test(NAME t00237
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-noangle.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-noangle.qasm.out 2>&1")
add_test(NAME t00238
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-qubit.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-qubit.qasm.out 2>&1")
add_test(NAME t00239
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-input.qasm > ${CMAKE_BINARY_DIR}/tests/test-input.qasm.out 2>&1")
add_test(NAME t00240
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-output.qasm > ${CMAKE_BINARY_DIR}/tests/test-output.qasm.out 2>&1")
add_test(NAME t00241
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-neg-trig.qasm > ${CMAKE_BINARY_DIR}/tests/test-neg-trig.qasm.out 2>&1")
add_test(NAME t00242
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-subscript-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-subscript-1.qasm.out 2>&1")
add_test(NAME t00243
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-frame-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-frame-1.qasm.out 2>&1")
add_test(NAME t00244
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-frame-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-frame-2.qasm.out 2>&1")
add_test(NAME t00245
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-frame-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-frame-3.qasm.out 2>&1")
add_test(NAME t00246
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cal-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-cal-1.qasm.out 2>&1")
add_test(NAME t00247
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cal-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-cal-2.qasm.out 2>&1")
add_test(NAME t00248
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cal-complex.qasm > ${CMAKE_BINARY_DIR}/tests/test-cal-complex.qasm.out 2>&1")
add_test(NAME t00249
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-dot-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-dot-1.qasm.out 2>&1")
add_test(NAME t00250
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-dot-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-dot-2.qasm.out 2>&1")
add_test(NAME t00251
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-logical-bang.qasm > ${CMAKE_BINARY_DIR}/tests/test-logical-bang.qasm.out 2>&1")
add_test(NAME t00252
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-waveform-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-waveform-1.qasm.out 2>&1")
add_test(NAME t00253
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-waveform-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-waveform-2.qasm.out 2>&1")
add_test(NAME t00254
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-waveform-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-waveform-3.qasm.out 2>&1")
add_test(NAME t00255
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-waveform-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-waveform-4.qasm.out 2>&1")
add_test(NAME t00256
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-waveform-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-waveform-5.qasm.out 2>&1")
add_test(NAME t00257
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pow-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-pow-1.qasm.out 2>&1")
add_test(NAME t00258
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pow-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-pow-2.qasm.out 2>&1")
add_test(NAME t00259
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pow-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-pow-3.qasm.out 2>&1")
add_test(NAME t00260
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pow-int.qasm > ${CMAKE_BINARY_DIR}/tests/test-pow-int.qasm.out 2>&1")
add_test(NAME t00261
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-play-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-play-1.qasm.out 2>&1")
add_test(NAME t00262
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-play-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-play-2.qasm.out 2>&1")
add_test(NAME t00263
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-builtins-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-builtins-1.qasm.out 2>&1")
add_test(NAME t00264
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-const-scalars.qasm > ${CMAKE_BINARY_DIR}/tests/test-const-scalars.qasm.out 2>&1")
add_test(NAME t00265
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pragma-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-pragma-1.qasm.out 2>&1")
add_test(NAME t00266
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pragma-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-pragma-2.qasm.out 2>&1")
add_test(NAME t00267
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-pragma-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-pragma-3.qasm.out 2>&1")
add_test(NAME t00268
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-run-defcal-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-run-defcal-1.qasm.out 2>&1")
add_test(NAME t00269
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-run-defcal-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-run-defcal-2.qasm.out 2>&1")
add_test(NAME t00270
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-identifier-overloading.qasm > ${CMAKE_BINARY_DIR}/tests/test-identifier-overloading.qasm.out 2>&1")
add_test(NAME t00271
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-cast-bool.qasm > ${CMAKE_BINARY_DIR}/tests/test-cast-bool.qasm.out 2>&1")
add_test(NAME t00272
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-c-comment.qasm > ${CMAKE_BINARY_DIR}/tests/test-c-comment.qasm.out 2>&1")
add_test(NAME t00273
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-conversion.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-conversion.qasm.out 2>&1")
add_test(NAME t00274
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-function-param-names.qasm > ${CMAKE_BINARY_DIR}/tests/test-function-param-names.qasm.out 2>&1")
add_test(NAME t00275
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-gphase.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-gphase.qasm.out 2>&1")
add_test(NAME t00276
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-noangle.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-noangle.qasm.out 2>&1")
add_test(NAME t00277
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-qubit.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-qubit.qasm.out 2>&1")
add_test(NAME t00278
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-idref-scope.qasm > ${CMAKE_BINARY_DIR}/tests/test-idref-scope.qasm.out 2>&1")
add_test(NAME t00279
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-if.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-if.qasm.out 2>&1")
add_test(NAME t00280
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-redeclarations.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-redeclarations.qasm.out 2>&1")
add_test(NAME t00281
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-2.qasm.out 2>&1")
add_test(NAME t00282
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-3.qasm.out 2>&1")
add_test(NAME t00283
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-return-xor.qasm > ${CMAKE_BINARY_DIR}/tests/test-return-xor.qasm.out 2>&1")
add_test(NAME t00284
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-ugate-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-ugate-3.qasm.out 2>&1")
add_test(NAME t00285
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-void-measure.qasm > ${CMAKE_BINARY_DIR}/tests/test-void-measure.qasm.out 2>&1")
add_test(NAME t00286
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-rz-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-rz-1.qasm.out 2>&1")
add_test(NAME t00287
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-rz-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-rz-2.qasm.out 2>&1")
add_test(NAME t00288
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-u-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-u-1.qasm.out 2>&1")
add_test(NAME t00289
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-consecutive-gates-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-consecutive-gates-1.qasm.out 2>&1")
add_test(NAME t00290
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-consecutive-gates-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-consecutive-gates-2.qasm.out 2>&1")
add_test(NAME t00291
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-switch-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-switch-1.qasm.out 2>&1")
add_test(NAME t00292
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-switch-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-switch-2.qasm.out 2>&1")
add_test(NAME t00293
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-switch-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-switch-3.qasm.out 2>&1")
add_test(NAME t00294
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-switch-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-switch-4.qasm.out 2>&1")
add_test(NAME t00295
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-do-while-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-do-while-1.qasm.out 2>&1")
add_test(NAME t00296
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-do-while-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-do-while-2.qasm.out 2>&1")
add_test(NAME t00297
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-do-while-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-do-while-3.qasm.out 2>&1")
add_test(NAME t00298
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-do-while-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-do-while-4.qasm.out 2>&1")
add_test(NAME t00299
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-do-while-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-do-while-5.qasm.out 2>&1")
add_test(NAME t00300
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-gate-switch.qasm > ${CMAKE_BINARY_DIR}/tests/test-gate-switch.qasm.out 2>&1")
add_test(NAME t00301
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bitset-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-bitset-cast.qasm.out 2>&1")
add_test(NAME t00302
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bool-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-bool-cast.qasm.out 2>&1")
add_test(NAME t00303
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-angle-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-angle-cast.qasm.out 2>&1")
add_test(NAME t00304
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bitset-int-cast.qasm > ${CMAKE_BINARY_DIR}/tests/test-bitset-int-cast.qasm.out 2>&1")
add_test(NAME t00305
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-gate-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-gate-1.qasm.out 2>&1")
add_test(NAME t00306
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-gate-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-gate-2.qasm.out 2>&1")
add_test(NAME t00307
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-gate-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-gate-3.qasm.out 2>&1")
add_test(NAME t00308
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-gate-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-gate-4.qasm.out 2>&1")
add_test(NAME t00309
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bit-gate-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-bit-gate-5.qasm.out 2>&1")
add_test(NAME t00310
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-unicode-epislon-gate.qasm > ${CMAKE_BINARY_DIR}/tests/test-unicode-epislon-gate.qasm.out 2>&1")
add_test(NAME t00311
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-annotation-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-annotation-1.qasm.out 2>&1")
add_test(NAME t00312
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-annotation-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-annotation-2.qasm.out 2>&1")
add_test(NAME t00313
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-while-loop-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-while-loop-1.qasm.out 2>&1")
add_test(NAME t00314
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-for-loop-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-for-loop-1.qasm.out 2>&1")
add_test(NAME t00315
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c-comment-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c-comment-1.qasm.out 2>&1")
add_test(NAME t00316
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c-comment-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c-comment-2.qasm.out 2>&1")
add_test(NAME t00317
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c-comment-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c-comment-3.qasm.out 2>&1")
add_test(NAME t00318
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c-comment-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c-comment-4.qasm.out 2>&1")
add_test(NAME t00319
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c++-comment-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c++-comment-1.qasm.out 2>&1")
add_test(NAME t00320
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c++-comment-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c++-comment-2.qasm.out 2>&1")
add_test(NAME t00321
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-c++-comment-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-c++-comment-3.qasm.out 2>&1")
add_test(NAME t00322
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-1.qasm.out 2>&1")
add_test(NAME t00323
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-2.qasm.out 2>&1")
add_test(NAME t00324
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-3.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-3.qasm.out 2>&1")
add_test(NAME t00325
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-4.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-4.qasm.out 2>&1")
add_test(NAME t00326
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-include-5.qasm > ${CMAKE_BINARY_DIR}/tests/test-include-5.qasm.out 2>&1")
add_test(NAME t00327
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-delay-1.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-delay-1.qasm.out 2>&1")
add_test(NAME t00328
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-defcal-delay-2.qasm > ${CMAKE_BINARY_DIR}/tests/test-defcal-delay-2.qasm.out 2>&1")
add_test(NAME t00329
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/adder_n4.qasm > ${CMAKE_BINARY_DIR}/tests/adder_n4.qasm.out 2>&1")
add_test(NAME t00330
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-multi-if.qasm > ${CMAKE_BINARY_DIR}/tests/test-multi-if.qasm.out 2>&1")
add_test(NAME t00331
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-multi-else.qasm > ${CMAKE_BINARY_DIR}/tests/test-multi-else.qasm.out 2>&1")
add_test(NAME t00332
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-multi-if-else.qasm > ${CMAKE_BINARY_DIR}/tests/test-multi-if-else.qasm.out 2>&1")
add_test(NAME t00333
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-sequence-elseif.qasm > ${CMAKE_BINARY_DIR}/tests/test-sequence-elseif.qasm.out 2>&1")
add_test(NAME t00334
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-multi-elseif-no-braces.qasm > ${CMAKE_BINARY_DIR}/tests/test-multi-elseif-no-braces.qasm.out 2>&1")
add_test(NAME t00335
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-nested-elseif.qasm > ${CMAKE_BINARY_DIR}/tests/test-nested-elseif.qasm.out 2>&1")
add_test(NAME t00336
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-multi-nested-elseif.qasm > ${CMAKE_BINARY_DIR}/tests/test-multi-nested-elseif.qasm.out 2>&1")
add_test(NAME t00337
         COMMAND ${BASH} -c "${OPENQASM_TEST_PROGRAM} -I${OPENQASM_TEST_INCDIR} ${OPENQASM_TEST_SRCDIR}/test-bitnot.qasm > ${CMAKE_BINARY_DIR}/tests/test-bitnot.qasm.out 2>&1")
