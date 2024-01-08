OPENQASM 3.0;

include "stdgates.inc";

qubit[16] q;

cphase(pi / 2) q[0], q[1];

// Two-argument
gate cv(theta, lambda) c, w {
  // implements controlled-U(theta, lambda) with target w and control c
  phase((lambda - theta) / 2) t;
  cx c, w;
  U(-theta / 2, 0, -lambda / 2) w;
  cx c, w;
  U(theta / 2, lambda, 0) w;
}

cv(pi / 2) q[2], q[3], q[4];

cu(pi / 2) q[5], q[6], q[7], q[8];
