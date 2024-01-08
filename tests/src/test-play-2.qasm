OPENQASM 3.0;

qubit $0;
qubit $1;

float[128] j = 1234567890.0987654321;

cal {
  extern port p0;
  waveform test_waveform = [1 + j im, 1 * j im, 1 / sqrt(2) + 1 / sqrt(2) * j im];
  frame test_frame = newframe(p0, 5e9, 0.0);
}

extern gaussian(complex[float[128]] a, duration d, duration s) -> waveform;

defcal xy(theta) $0 {
  play(test_waveform, test_frame);
}

complex[float[128]] ampl;

duration dur = 150ns;

duration sigma = 300ns;

defcal xz(phi) $1 {
  frame f1 = newframe(p0, 15e10, 1.0);
  play(gaussian(ampl, dur, sigma), f1);
}

bit[2] c;

measure $0 -> c[0];
measure $1 -> c[1];
