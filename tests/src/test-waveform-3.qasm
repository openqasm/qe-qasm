OPENQASM 3.0;

extern gaussian_square(complex[float[128]] amp, duration d, duration sqw,
                       duration sigma) -> waveform;

cal {
  waveform wf = gaussian_square(1.0, 16000ms, 262ns, 13952μs);
}
