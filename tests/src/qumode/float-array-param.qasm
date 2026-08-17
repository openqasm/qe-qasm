OPENQASM 3.0;

gate rz3(array[float[64], 3] thetas) qubit q {
  ctrl @ gphase(thetas[0]) q;
  ctrl @ gphase(thetas[1]) q;
  ctrl @ gphase(thetas[2]) q;
}

qubit[3] q;

rz3([pi/2, 0, 0.3]) q[0];
