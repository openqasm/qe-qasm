OPENQASM 3.0;

gate x a {
  U(π, 0, π) a;
}

gate cx a, b {
  ctrl @ x a, b;
}

gate rx(theta) a {
  U(theta, -pi / 2, pi / 2) a;
}

gate phase(λ) q {
  U(0, 0, λ) q;
}

gate cu(theta, phi, lambda) c, t {
  phase((lambda - phi) / 2) t;
  cx c, t;
  U(-theta / 2, 0, -(phi + lambda) / 2) t;
  cx c, t;
  U(theta / 2, phi, 0) t;
}

