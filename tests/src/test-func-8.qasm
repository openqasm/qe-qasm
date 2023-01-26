OPENQASM 3.0;

gate phase(lambda) q {
  U(0, 0, lambda) q;
}

gate cx c, t {
  CX c, t;
}

gate t a {
  phase(pi/4) a;
}

gate tdg a {
  phase(-pi/4) a;
}

gate h a {
  U(pi / 2, 0, pi) a;
}

gate s a {
  phase(pi / 2) a;
}

gate z a {
  phase(pi) a;
}

gate ccx a, b, c {
  h c;
  cx b, c;
  tdg c;
  cx a, c;
  t c;
  cx b, c;
  tdg c;
  cx a, c;
  t b; t c; h c;
  cx a, b;
  t a; tdg b;
  cx a, b;
}

def segment (qubit[2] anc, qubit psi) -> bit[2] {
  bit[2] b;
  reset anc;
  h anc;
  ccx anc[0], anc[1], psi;
  s psi;
  ccx anc[0], anc[1], psi;
  z psi;
  h anc;
  measure anc -> b;
  return b;
}

