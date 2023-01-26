OPENQASM 3.0;

gate p(λ) a {
  ctrl @ gphase(λ) a;
}

gate z a {
  p(π) a;
}

gate s a {
  pow(1/2) @ z a;
}

gate tdg a {
  inv @ pow(1/2) @ s a;
}

gate stdg a {
  inv @ pow(1/2) @ tdg a;
  inv @ pow(1/4) @ tdg a;
  inv @ pow(1/8) @ tdg a;
}

gate ctdg a, b {
  ctrl @ pow(1/2) @ tdg a, b;
  ctrl @ pow(1/4) @ tdg a, b;
  ctrl @ pow(1/8) @ tdg a, b;
}

gate wtdg(λ) a, b {
  ctrl @ pow(1/2) @ p(λ) a, b;
  ctrl @ pow(1/2) @ p(λ) a, b;
  ctrl @ pow(1/2) @ p(λ) a, b;
}

