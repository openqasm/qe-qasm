OPENQASM 3.0;

gate greek(λ) a {
  U(π, 0.0, π) a;
}

angle[32] λ;

gate roman(λ) q {
  U(π, 0.0, π) q;
}
