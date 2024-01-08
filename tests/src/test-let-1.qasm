OPENQASM 3.0;

// Qubit declarations
qubit[2] one;
qubit[10] two;

// Aliased register of twelve qubits
let concatenated = one ++ two;
