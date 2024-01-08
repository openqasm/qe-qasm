OPENQASM 3.0;

include "stdgates.inc";

// 10. scheduling policy
// Prior to this scheduling pass, the scheduling policy is not defined.
// The program just means we apply these operations in any order
// consistent with the data flow. The program is not really executable yet.

// I'd like this pass to produce an implicitly-timed program that can
// run on any backend with (cx, phase, h) gates where physical qubits
// 0 and 1 are adjacent. I'd like to express a scheduling policy using
// stretch types and delays with stretchable durations that can be solved
// explicitly later by the "stretch solver".

qubit $0;
qubit $1;

angle[3] c = 0;
reset $0;
reset $1;
h $1;
h $0;

stretch a;
stretch b;

delay(a) $0;
delay(b) $1;
cx $0, $1;
phase(1.8125 * pi) $1;
cx $0, $1;
phase(0.1875 * pi) $1;
phase(0.1875 * pi) $0;
h $0;
measure $0 -> c[0];
c <<= 1;
reset $0;
h $0;
stretch k;
delay(k) $1;
cx $0, $1;
phase(1.625 * pi) $1;         // mod 2*pi
cx $0, $1;
phase(0.375 * pi) $1;
angle[32] temp_1 = 0.375 * pi;
temp_1 -= c;                  // cast and do arithmetic mod 2 pi
phase(temp_1) $0;
h $0;
measure $0 -> c[0];
c <<= 1;
reset $0;
h $0;
stretch d;
delay(d) $1;
cx $0, $1;
phase(1.25 * pi) $1;          // mod 2*pi
cx $0, $1;
phase(0.75 * pi) $1;
angle[32] temp_2 = 0.75*pi;
temp_2 -= c;                  // cast and do arithmetic mod 2 pi
phase(temp_2) $0;
h $0;
measure $0 -> c[0];
c <<= 1;
stretch f;
delay(f) $1;
