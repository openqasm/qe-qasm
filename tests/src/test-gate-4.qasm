OPENQASM 3.0;

gate test(angle[48] theta, angle[48] phi, angle[48] sigma) q, r, s {
  U(theta, 0.0, pi) q;
  U(phi, 0.0, pi / 2) r;
  U(sigma, 0.0, -pi / 4) s;
}
