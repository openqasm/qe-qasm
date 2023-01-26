OPENQASM 3.0;

qubit $0;

gate test(lambda) q, r, s {
  U(lambda, 0.0, 3.14159265359) q;
  U(lambda, 0.0, 3.14159265359) r;
  U(lambda, 0.0, 3.14159265359) s;
}

