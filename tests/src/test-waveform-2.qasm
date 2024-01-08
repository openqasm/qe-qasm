OPENQASM 3.0;

complex[float[128]] cxa = 5.25  * 1 im;

complex[float[128]] cxb = 15.25 * 1 im;

complex[float[128]] cxc = 25.25 * 1 im;

cal {
  waveform waveform1 = [cxa, cxb, cxc];
}
