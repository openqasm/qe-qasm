OPENQASM 3.0;

gate p(λ) a { ctrl @ gphase(λ) a; }

gate z a { p(π) a; }

gate s a { pow(1/2) @ z a; }

gate tdg a { inv @ pow(1/2) @ s a; }
