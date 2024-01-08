OPENQASM 3.0;

include "stdgates.inc";

qubit q[4];

gate rz(tau) q {
  gphase(-tau / 2);
  U(0, 0, tau) q;
}

ctrl @ rz(pi / 2) q[1], q[0];
