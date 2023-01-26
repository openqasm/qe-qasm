OPENQASM 3.0;

uint n = 2;

def parity(bit[n] cin) -> bit {
  bit c;
  for i in [0:2:100] {
    c ^= cin[i];
  }

  return c;
}

def func_calls_parity(float f, int k, bit b) -> int {
  bit[2] k;
  return parity(k);
}

bit result;
qubit q;
qubit r;

bit bq = measure q;
bit br = measure r;

result = parity(bq || br);

float a0 = pi;

int a1 = 20;

int result2;

result2 = func_calls_parity(a0, a1, parity(bq || br));

