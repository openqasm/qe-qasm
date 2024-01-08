OPENQASM 3.0;

qubit[2] q;

gate cx c, t {
  CX c, t;
}

box bx {
  cx q[0], q[1];
  reset q;
}

duration d = durationof(bx);
