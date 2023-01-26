OPENQASM 3.0;

uint n = 8;

def parity(bit[n]:cin) -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

def both(bit[n]:cin, angle[22]:ain, float:fin) qubit[4]:d, qubit:a -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

