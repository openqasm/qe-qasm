OPENQASM 3.0;

gate rz3(array[float[64], 3] thetas) qubit q {
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
  ctrl @ gphase(thetas[2]) q;
}

qubit q;

// Negative: formal size 3, literal size 2.
rz3([pi/2, 0]) q;
