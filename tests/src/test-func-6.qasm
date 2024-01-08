OPENQASM 3.0;

def quantum_params(qubit a) -> bit {
  return measure a;
}

def classical_params(int i, float f) -> float {
  return i * f;
}

def mixed_params(int i, qubit b) -> bit {
  return measure b;
}
