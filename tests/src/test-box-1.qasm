OPENQASM 3.0;

include "stdgates.inc";

qubit[8] q;

box test_box_1 {
  cx q[0], q[1];
  reset q;
}

