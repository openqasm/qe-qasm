OPENQASM 2.0;

include "qelib1.inc";

qreg qubits[32];

// 1. Defcal declaration with parameter and qubit argument..
defcal rz(angle[20]:theta) %q {  }

// 2. Defcal declaration with parameter and qubit argument.
defcal rx(angle[20]:theta) %0 { }

// 3. Defcal declaration with measure.
defcal measure %q -> bit {  }

// 4. Defcal measure declaration with implicit grammar declaration.
defcal "openpulse" measure %q -> bit { }

// 5. Defcal declaration with no parameters.
defcal d %0 { }

// 6. Defcal declaration with no parameters.
defcal d %1 { }

// 7. Defcal declaration with implicit (pi) parameter.
defcal rx(pi) %0 { }

// 8. Defcal declaration with expression parameter based on pi.
defcal rx(pi / 2) %0 { }

// 9. Defcalgrammar declaration for "openpulse".
defcalgrammar "openpulse";
