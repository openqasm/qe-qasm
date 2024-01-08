OPENQASM 3.0;

gate x(theta, phi) q, t {
  U(theta, 0.0, phi) q;
  U(theta, 0.0, phi) t;
}

def xfunction(int x) -> int {
  return x + 5;
}

def yfunction(int y) -> int {
  int x = 3;
  return y + x;
}
