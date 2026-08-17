OPENQASM 3.0;

// Fock level may be an expression (e.g. N-1), not only a bare id/literal.
gate foo[uint N](array[float[64], N] thetas) qumode qm {
  ctrl[N-1] @ gphase(pi/2) qm;
  ctrl[(N-1)*2/3] @ gphase(pi/4) qm;
}

qumode qm;
foo[3]([0.1, 0.2, 0.3]) qm;
