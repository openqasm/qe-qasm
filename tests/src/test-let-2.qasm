OPENQASM 3.0;

// 1. Qubit array declaration.
qubit[10] ten;

// 2. Another qubit array declaration.
qubit[20] twenty;

// 3. First qubit in qubit array.
let first = ten[0];

// 4. Last qubit in qubit array.
let last = twenty[-1];

