OPENQASM 3.0;

// Positive: array[angle, N] formals support body indexing (like float arrays).
gate rz3a(array[angle, 3] thetas) qubit q {
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
  ctrl @ gphase(thetas[2]) q;
}

// Scalar angle before array formal; second gate reuses thetas[i] ASTM keys.
gate rz3b(angle alpha, array[angle, 3] thetas) qubit q {
  ctrl @ gphase(alpha) q;
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
  ctrl @ gphase(thetas[2]) q;
}

qubit[3] q;

rz3a([pi/2, 0, 0.3]) q[0];
rz3b(pi/4, [pi/2, 0, 0.3]) q[0];
