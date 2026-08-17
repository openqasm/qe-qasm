OPENQASM 3.0;

// Template N never appears as an array size — nothing to infer from.
gate foo[uint N](angle alpha) qubit q {
}

qubit q;
foo(pi/2) q;
