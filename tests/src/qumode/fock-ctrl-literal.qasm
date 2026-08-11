OPENQASM 3.0;

gate fock_phase(angle a) qumode qm {
  ctrl[3] @ gphase(a) qm;
}

qumode qm;
fock_phase(pi/2) qm;
