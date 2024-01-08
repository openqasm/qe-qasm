OPENQASM 3.0;

cal {
  extern port p0;
  frame test_frame = newframe(p0, 5e9, 0.0);

  complex[float[64]] c;
  c.creal = 75.25;
  c.cimag = 31.231;
}

complex[float[64]] c;
c.creal = 75.25;
c.cimag = 31.231;
