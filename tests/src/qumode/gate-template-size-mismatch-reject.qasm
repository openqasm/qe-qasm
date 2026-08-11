OPENQASM 3.0;

gate foo[uint N](array[float[64], N] thetas) qubit q {
}

qubit q;
foo[2]([pi/2, 0, 0.3]) q;
