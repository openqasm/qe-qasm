OPENQASM 3.0;

include "cvgates.inc";

// Gate-body for with Fock ctrl and template range end.
gate snap_body[uint N](array[float[64], N] thetas) qumode qm {
  for i in [0:N] {
    ctrl[i] @ gphase(thetas[i]) qm;
  }
}

qumode qm;
snap_body[3]([0.1, 0.2, 0.3]) qm;
