OPENQASM 3.0;

gate x a {
  U(π, 0, π) a;
}

gate phase(lambda) q {
  U(0, 0, lambda) q;
}

gate rz(phi) a {
  phase(phi) a;
}

gate rx(theta) a {
  U(theta, -pi / 2, pi / 2) a;
}

qubit q;

rx(2 * π / 12) q;

box box_1 {
  delay[dt] q;
  x q;
  delay[dt] q;
  x q;
  delay[dt] q;
}

rx(3 * π / 12) q;

rx(5 * π / 12) q;

box box_2 {
  delay[dt] q;
  x q;
  delay[dt] q;
  x q;
  delay[dt] q;
}

