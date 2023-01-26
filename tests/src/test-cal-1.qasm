OPENQASM 3.0;

cal {
  extern port p0;
  frame test_frame = newframe(p0, 5e9, 0.0);
}

cal {
  extern port e0;
  frame inner_frame = newframe(e0, 5e10, pi);

  inner_frame.freq = 35e10;
  inner_frame.time = 100μs;
  inner_frame.phase = tau / 4;
}

