OPENQASM 3.0;

// 0. Defcalgrammar declaration for "openpulse".
defcalgrammar "openpulse";

qubit $0;
qubit $1;
qubit $2;
qubit[8] $3;

uint n = 64U;

// 1. Defcal declaration with parameter and qubit argument.
defcal rz(theta) $2 { }

// 2. Defcal declaration with parameter and qubit argument.
defcal rx(theta) $3[0] { }

// 3. Defcal declaration with measure.
defcal measure $2 -> bit { }

// 4. Defcal measure declaration with implicit grammar declaration.
defcal "openpulse" measure $1 -> bit { }

// 5. Defcal declaration with no parameters.
defcal d $0 { }

// 6. Defcal declaration with no parameters.
defcal d $1 { }

// 7. Defcal declaration with implicit (pi) parameter.
defcal rx(pi) $0 { }

// 8. Defcal declaration with expression parameter based on pi.
defcal rx(pi / 2) $0 { }

// 9. Defcal declaration with expression parameter based on pi.
defcal ry(-pi / 2) $0 { }

// 10. Defcal declaration with two qubits.
defcal two_qubit_gate $0 $1 { }

// 11. Defcal declaration with two angle parameters and two qubits.
defcal two_params(theta, pi) $0, $1 { }

// 11. Defcal declaration with three angle parameters and two qubits.
defcal three_params(theta, pi, sigma) $0, $1 { }

// 12. Defcal declaration with an indexed Qubit from a QubitContainer.
defcal mx(lambda) $3[5] { }
