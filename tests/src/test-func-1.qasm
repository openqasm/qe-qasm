OPENQASM 3.0;

gate x a { U(π, 0, π) a; }

gate cx c, t { ctrl @ x c, t; }

def xcheck(qubit[4] d, qubit a)  -> bit {
  reset a;
  for i in [0:3]
    cx d[i], a;

  return measure a;
}

uint n = 32;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:32] {
    c ^= cin[i];
  }

  return c;
}

