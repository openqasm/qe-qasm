OPENQASM 3.0;

def test(qubit[2] anc) -> bit[2] {
  bit[2] r;
  reset anc;
  return r;
}

