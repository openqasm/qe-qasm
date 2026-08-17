OPENQASM 3.0;

// Named array is declared with size 3 (enough to infer N) but never assigned.
// Expect: "Variable 'a' used before assigned."
gate foo[uint N](array[float[64], N] thetas) qubit q {
}

array[float[64], 3] a;
qubit q;
foo(a) q;
