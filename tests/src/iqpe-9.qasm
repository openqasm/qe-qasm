OPENQASM 3.0;

include "stdgates.inc";

// 9. Layout on physical qubits
// Map q and r to physical qubits 0 and 1 respectively (assume
// they are adjacent, but in general we would insert swaps or
// other communication instructions at this step):

qubit $0;
qubit $1;

angle[3] c = 0;

reset $0;
reset $1;

h $1;
h $0;
cx $0, $1;
phase(1.8125 * pi) $1;        // mod 2*pi
cx $0, $1;
phase(0.1875 * pi) $1;
phase(0.1875 * pi) $0;
h $0;

measure $0 -> c[0];

c <<= 1;
reset $0;

h $0;
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
cx $0, $1;
phase(1.25 * pi) $1;          // mod 2*pi
cx $0, $1;
phase(0.75 * pi) $1;

angle[32] temp_2 = 0.75 * pi;

temp_2 -= c;                  // cast and do arithmetic mod 2 pi
phase(temp_2) $0;
h $0;
measure $0 -> c[0];
c <<= 1;
