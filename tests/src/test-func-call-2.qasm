OPENQASM 3.0;

uint n = 8;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

bit result;
qubit q;
qubit r;

bit bq = measure q;
bit br = measure r;

result = parity(bq || br);

