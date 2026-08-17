OPENQASM 3.0;

// Template param N visible in body as Fock level (literal/identifier only).
gate foo[uint N](array[float[64], N] thetas) qumode qm {
  ctrl[N] @ gphase(pi/2) qm;
}

qumode qm;
foo[3]([0.1, 0.2, 0.3]) qm;
