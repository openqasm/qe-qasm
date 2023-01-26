OPENQASM 3.0;

gate test_gate(theta, lambda, phi) q {
  U(-pi, theta, -lambda) q;
}

double theta = pi * 4;

def function(int sigma, double theta, angle[64] phi) -> int {
  float[64] lambda = 1.5;
  float[64] rho = lambda / pi;

  return 0;
}

