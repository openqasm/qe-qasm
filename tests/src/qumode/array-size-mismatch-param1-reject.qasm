OPENQASM 3.0;

gate baz(array[angle, 3] alphas, array[angle, 2] thetas) qubit q {
  ctrl @ gphase(alphas[0]) q;
  ctrl @ gphase(thetas[0]) q;
}

qubit q;

// Negative: second formal expects size 2, literal has size 3.
baz([pi/2, 0, 0.3], [pi/4, 0.7, 0.2]) q;
