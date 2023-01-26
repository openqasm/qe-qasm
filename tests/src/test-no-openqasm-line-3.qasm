/* This is a C comment. */

// This is a C++ comment.

/*
 * This is a multi-line C comment.
 * This is the second line.
 * This is the third line.
 */

uint j = 64;

cal {
  waveform wf0 = [1 + j im, j + 3 im, 1 / sqrt(2) + 1 / sqrt(2) * j im];

  complex[float[128]] a = 5.25  * 1 im;

  complex[float[128]] b = 15.25 * 1 im;

  complex[float[128]] c = 25.25 * 1 im;

  waveform wf1 = [a, b, c];
}

