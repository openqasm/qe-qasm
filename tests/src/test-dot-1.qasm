OPENQASM 3.0;

cal {
  extern port p0;

  frame foo = newframe(p0, 5e9, 0.0);

  foo.frequency = 10;

  foo.time = 15;

  foo.phase = 25;
}

