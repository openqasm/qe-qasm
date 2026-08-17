OPENQASM 3.0;

// Explicit `angle` is a valid fully-typed classical formal (alongside arrays).
gate foo(array[float[64], 3] thetas, angle alpha) qubit q {
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
  ctrl @ gphase(thetas[2]) q;
  ctrl @ gphase(alpha) q;
}

gate bar(angle alpha) qubit q {
  ctrl @ gphase(alpha) q;
}

qubit q;

bar(pi/4) q;
foo([pi/2, 0, 0.3], pi/4) q;
