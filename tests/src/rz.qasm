OPENQASM 3.0;

include "stdgates.inc";

qubit xq[4];

gate rz(tau) xq {
  gphase(-tau / 2);
  U(0, 0, tau) xq;
}

rz(pi / 2) xq[1], xq[0];

