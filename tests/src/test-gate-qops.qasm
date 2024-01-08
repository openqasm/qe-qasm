OPENQASM 3.0;

gate phase(lambda) q {
  U(0, 0, lambda) q;
}

gate extra(lambda, theta, phi) q {
  U(phi, theta, lambda) q;
  phase(lambda) q;
  U(phi, theta, lambda) q;
  phase(lambda) q;
}

angle[32] X = 1.0;
angle[32] Y = 2.0;
angle[32] Z = 3.0;

qubit $0;
qubit $1;

phase(X) $0;

extra(X, Y, Z) $1;
