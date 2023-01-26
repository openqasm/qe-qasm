OPENQASM 3.0;

@one
gate p(λ) a { ctrl @ gphase(λ) a; }

@foo this is an annotation
gate z a { p(π) a; }

@bind ioport
gate s a { pow(1/2) @ z a; }

@reversible
gate tdg a { inv @ pow(1/2) @ s a; }

