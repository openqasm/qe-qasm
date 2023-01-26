OPENQASM 3.0;

gate cx c, t {
  CX c, t;
}

uint n = 16;

def funcboth(bit[n] cin, angle[24] ang, float fin, qubit[4] d, qubit a) -> bit {
  reset a;
  for i in [0:3]
    cx d[i], a;

  bit c;
  for i in [0:2:16] {
    c ^= cin[i];
  }

  if (c == 1)
    return c;

  return measure a;
}

