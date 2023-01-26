OPENQASM 3.0;

defcalgrammar "openpulse";

extern gaussian(complex[float[128]] a, duration d, duration s) -> waveform;

float gfreq = 5.9e9;

cal {
  // Defined within `cal`, so it may not leak back out to the enclosing blocks scope
  float nfreq = 5.2e9;

  // declare global port
  extern port d0;

  // reference `freq` variable from enclosing blocks scope
  frame d0f = newframe(d0, nfreq, 0.0);
}

defcal x $0 {
  waveform xp = gaussian(1.0, 160dt, 40dt);

  // References frame and `nfreq` declared in top-level cal block
  play(xp, d0f);

  d0f.frequency = nfreq;
  play(xp, d0f);
}

qubit $1;
qubit $2;

x $1;
x $2;

