OPENQASM 3.0;

gate foo(theta, phi) q, r {
  U(0.0, theta, phi) q;
  U(0.0, theta, phi) r;
  reset q;
}
