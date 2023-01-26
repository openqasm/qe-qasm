OPENQASM 3.0;

include "stdgates.inc";

qubit[8] q;

boxas test_box_2 {
  cx q[0], q[1];
  delay[200ns] q[0];
}

delay[durationof(test_box_2)] q[2], q[3];
cx q[2], q[3];

