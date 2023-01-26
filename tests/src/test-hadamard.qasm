OPENQASM 3.0;

// Clifford gate: Hadamard
gate h a {
  U(pi / 2, 0, pi) a;
}

