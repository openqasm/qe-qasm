OPENQASM 3.0;

gate test q, r {
  U(pi, 0.0, pi / 2) q;
  U(pi, 0.0, pi / 2) r;
}

qubit q;
qubit r;

