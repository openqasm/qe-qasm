OPENQASM 2.0;

// --- QE Hardware primitives ---

// 3-parameter 2-pulse single qubit gate
gate u3(θ,φ,λ) q { U(θ,φ,λ) q; }

// 2-parameter 1-pulse single qubit gate
gate u2(φ,λ) q { U(π/2,φ,λ) q; }

// 1-parameter 0-pulse single qubit gate
gate u1(λ) q { U(0,0,λ) q; }

// Pauli gate: bit-flip
gate x a { u3(π,0,π) a; }

// Pauli gate: bit and phase flip
gate y a { u3(π,π/2,π/2) a; }

// Pauli gate: phase flip
gate z a { u1(π) a; }

// Clifford gate: Hadamard
gate h a { u2(0,π) a; }

// Clifford gate: sqrt(Z) phase gate
gate s a { u1(π/2) a; }

// Clifford gate: conjugate of sqrt(Z)
gate sdg a { u1(-π/2) a; }

// C3 gate: sqrt(S) phase gate
gate t a { u1(π/4) a; }

// C3 gate: conjugate of sqrt(S)
gate tdg a { u1(-π/4) a; }

if (tdg == a) {
  u1(-π / 4) a;
} else {
  u1(-π / 2) a;
}

if (tdg == a) {
  u1(-π / 4) a;
  CX c,t;
  u3(π,0,π) a;
} else {
  u1(-π / 2) a;
  u3(π,0,π) a;
}

