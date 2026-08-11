OPENQASM 3.0;

// Positive: two angle-array formals with different sizes.
gate baz(array[angle, 3] alphas, array[angle, 2] thetas) qubit q {
  ctrl @ gphase(alphas[0]) q;
  ctrl @ gphase(alphas[1]) q;
  ctrl @ gphase(alphas[2]) q;
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
}

qubit q;

baz([pi/2, 0, 0.3], [pi/4, 0.7]) q;
