OPENQASM 3.0;

include "stdgates.inc"

// 1. Fixed length, in standard units
length a = 300ns;

// 2. Fixed length, backend dependent
length b = 800dt;

// 3. Fixed length, referencing the duration of a calibrated gate
length c = lengthof(h);

// 4. Dynamic length, referencing a box within its context
qubit[8] q;

box bx {
  cx q[0], q[1];
  reset q;
}

length d = lengthof(bx);

// 5. Length initialization list
length la, lb, lc, ld = 500ns;

