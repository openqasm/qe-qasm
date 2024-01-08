OPENQASM 3.0;

uint j = 64;

cal {
  waveform wf0 = [1 + j im, j + 3 im, 1 / sqrt(2) + 1 / sqrt(2) * j im];

  complex[float[128]] a = 5.25  * 1 im;
  complex[float[128]] b = 15.25 * 1 im;
  complex[float[128]] c = 25.25 * 1 im;

  waveform wf1 = [a, b, c];
  waveform wfm = mix(wf0, wf1);
  waveform wfs = sum(wfm, wf1);
}

angle[64] an = pi / 2;

cal {
  waveform wfp = phase_shift(wfs, an);
  float[64] fct = 0.9137815;
}

cal {
  waveform wfc = scale(wfp, fct);
}
