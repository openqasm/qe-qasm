OPENQASM 3.0;

// Template uint N as array length; call-site inference from literal arity.
gate foo[uint N](array[float[64], N] thetas) qubit q {
}

qubit q;
foo([pi/2, 0, 0.3]) q;
foo[3]([pi/2, 0, 0.3]) q;
