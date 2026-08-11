OPENQASM 3.0;

gate g(array[float[64], 3] thetas) qubit q {
  ctrl @ gphase(thetas[0]) q;
}

qubit q;

// Negative: float/angle array elements must be real, not complex.
g([pi/2, 0, 0.3 + 0.5 im]) q;
