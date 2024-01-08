OPENQASM 3.0;

gate test_gate(theta) q, r {
  U(pi / 2, pi, -pi) q;
  U(pi / 2, pi, -pi / 4) r;
}

cal {
  extern port p0;
  extern frame extern_frame;

  frame test_frame = newframe(p0, 5e9, 0.0);

  test_frame.phase = pi / 4;

  test_frame.frequency = 35e10;

  test_frame.time = 100ns;
}

qubit $0;
qubit $1;

test_gate $0, $1;
