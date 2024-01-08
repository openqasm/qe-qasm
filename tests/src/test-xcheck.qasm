OPENQASM 3.0;

gate cx a, b { }

def xcheck(qubit[4] d, qubit a) -> bit {
  reset a;
  for i in [0:3]
    cx d[i], a;

  return measure a;
}
