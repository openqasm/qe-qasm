OPENQASM 3.0;

// Qubit declarations
qubit[2] one;
qubit[10] two;

// Aliased register of twelve qubits
let concatenated = one ++ two;

// First qubit in aliased qubit array
let first = concatenated[0];

// Last qubit in aliased qubit array
let last = concatenated[-1];

// Qubits zero, three and five
let qubit_selection = two[0, 3, 5];

// First six qubits in aliased qubit array
let sliced = concatenated[0:6];

// Every second qubit
let every_second = concatenated[0:2:11];

// Using negative ranges to take the last 3 elements
let last_three = two[-4:-1];

// Concatenate two alias in another one
let both = sliced ++ last_three;

