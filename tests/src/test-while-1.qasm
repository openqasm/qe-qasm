OPENQASM 3.0;

include "stdgates.inc";

qubit q;
qubit r;

angle[3] c = 0;

reset q;
reset r;
h r;

uint[3] power = 1;

uint n = 125;

// While loop.
while (n >= 0) {
  reset q;
  h q;
  cphase(power*3*pi/8) q, r;
  phase(-c) q;
  h q;
  measure q -> c[0];
  c <<= 1;
  power <<= 1;
  --n;
}
