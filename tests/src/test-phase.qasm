OPENQASM 3.0;

// phase gate (Z-rotation by lambda)
gate phase(lambda) q {
  U(0, 0, lambda) q;
}

