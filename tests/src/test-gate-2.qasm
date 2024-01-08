OPENQASM 3.0;

gate phase(lambda) q {
  U(0, 0, lambda) q;
}

gate extra(lambda, theta, phi) q {
  U(phi, theta, lambda) q;
}
