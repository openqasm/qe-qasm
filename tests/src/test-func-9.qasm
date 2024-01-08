OPENQASM 3.0;

uint n = 32;

def parity(bit[n] cin, ...) -> bit {
  bit c;
  for i in [0:2:32] {
    c ^= cin[i];
  }

  return c;
}

def trueorfalse(int i, float f, angle[64] a) -> bool {
  if (i < 3)
    return true;
  else
    return false;
}
