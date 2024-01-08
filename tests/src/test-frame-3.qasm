OPENQASM 3.0;

include "stdopenpulse.inc";

qubit $0;
qubit $1;

cal {
  extern port p0;
  extern port p1;

  frame driveframe1 = newframe(p0, 5.1e9, 0.0);
  frame driveframe2 = newframe(p1, 5.2e9, 0.0);
}

defcal test_defcal(theta, lambda, phi) $0, $1 {
  delay[15ns] driveframe1;
  delay[110μs] driveframe2;

  set_phase(driveframe1, pi);
  set_frequency(driveframe2, 0.0210315);
}
