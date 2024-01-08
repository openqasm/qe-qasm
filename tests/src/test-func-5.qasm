OPENQASM 3.0;

include "stdgates.inc";

def xcheck(qubit[4] d, qubit a) -> bit {
  reset a;
  for i in [0:3]
    cx d[i], a;

  return measure a;
}

uint n = 100;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

def both(bit[n] cin, angle[24] ang, float fin, qubit[4] d, qubit a) -> bit {
  reset a;
  for i in [0:3]
    cx d[i], a;

  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  if (c == 1)
    return c;

  return measure a;
}
