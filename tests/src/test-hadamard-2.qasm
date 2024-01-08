OPENQASM 3.0;

// Clifford gate: Hadamard
gate h a {
  U(pi / 2, 0, pi) a;
  U(2 * pi, 0, pi) a;
}
