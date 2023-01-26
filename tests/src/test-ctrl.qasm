OPENQASM 3.0;

gate phase(lambda) q, r {
  U(0, 0, lambda) q;
  U(0, 0, lambda) r;
}

const int n = 128;

uint[n] power = 1;

angle theta = pi / 2;

ctrl @ phase(theta) q, r;

pow(n) @ phase(theta) q, r;

gate crz(phi) q1, q2 {
  ctrl @ U(phi, 0, 0) q1, q2;
}

