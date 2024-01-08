OPENQASM 3.0;

qubit $0;

length a = 300ns;

length b = lengthof($0);

stretch c;

// stretchy length with min=300ns
length d = a + 2 * c;

// stretchy length with backtracking by up to half b
length e = -0.5 * b + c;
