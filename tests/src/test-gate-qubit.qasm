OPENQASM 3.0;

gate test(theta, phi) q, r {
  U(theta, 0.0, phi) q;
  U(theta, 0.0, phi) r;
}

qubit q;
qubit r;
