OPENQASM 3.0;

gate foo(pi) q {
  U(pi, 0, pi) q;
}

gate bar(tau) q {
  U(tau, 0, tau) q;
}

gate baz(euler) q {
  U (euler, 0, euler) q;
}
