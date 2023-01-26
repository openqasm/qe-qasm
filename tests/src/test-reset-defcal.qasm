OPENQASM 3.0;

qubit $0;
qubit $1;

gate foo(theta, phi) q, r {
  U(0.0, theta, phi) q;
  U(0.0, theta, phi) r;
}

defcal foo(theta, phi) $0, $1 {
  U(0.0, theta, phi) $0;
  U(0.0, theta, phi) $1;

  measure $0;
  measure $1;

  reset $0;
  reset $1;
}

