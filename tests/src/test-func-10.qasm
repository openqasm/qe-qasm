OPENQASM 3.0;

qubit q;
qubit r;

def noargs() -> bit {
  return measure q;
}

def noargs_noparens -> bit {
  return measure r;
}

def noargs_noresult() {
  return;
}

def args_noresult(int i, int j) {
  int k = i + j;
  return;
}

