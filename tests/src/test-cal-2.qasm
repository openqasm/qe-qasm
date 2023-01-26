OPENQASM 3.0;

// Use a boxcar function to generate IQ data from raw waveform
extern boxcar(waveform iwf) -> complex[float[64]];

// Use a linear discriminator to generate bits from IQ data
extern discriminate(complex[float[64]] iq) -> bit;

extern gaussian_square(complex[float[128]] amp, duration d, duration sqw,
                       duration sigma) -> waveform;

cal {
  // Define two ports
  extern port m0;
  extern port cap0;
}

qubit $0;

defcal measure $0 -> bit {
  // Force time of carrier to 0 for consistent phase for discrimination.
  frame stimulus_frame = newframe(m0, 5e9, 0.0);
  frame capture_frame = newframe(cap0, 5e9, 0.0);

  duration d0 = 16000dt;
  duration d1 = 262dt;
  duration d2 = 13952dt;

  // Measurement stimulus envelope
  waveform meas_wf = gaussian_square(1.0, d0, d1, d2);

  // Play the stimulus
  play(meas_wf, stimulus_frame);

  // Align measure and capture frames
  barrier stimulus_frame, capture_frame;

  // Capture transmitted data after interaction with measurement resonator
  extern capture(duration d, frame cf) -> waveform;

  waveform raw_output = capture(d0, capture_frame);

  // Kernel and discriminate
  complex[float[32]] iq = boxcar(raw_output);
  bit result = discriminate(iq);

  return result;
}

