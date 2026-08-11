OPENQASM 3.0;

// Same template N on two array formals; conflicting literal arities.
gate foo[uint N](array[angle, N] alphas, array[angle, N] betas) qubit q {
}

qubit q;
foo([pi/2, pi/2, pi/2], [pi/2, pi/2]) q;
