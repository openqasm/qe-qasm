OPENQASM 3.0;

// Qubit declarations
qubit[2] one;
qubit[10] two;
qubit[8] three;

// Aliased register of twelve qubits
let concatenated = one || two;

// Another aliased register of eighteen qubits
let glued = two ++ three;

