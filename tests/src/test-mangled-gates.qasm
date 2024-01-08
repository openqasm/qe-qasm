OPENQASM 3.0;

gate x a { U(π, 0, π) a; }

gate rx(θ) a { U(θ, -π/2, π/2) a; }

gate triple(θ, μ, γ) r, s, t {
  U(θ, -π/2, π/2) r;
  U(-π/2, μ, π/2) s;
  U(-π/2, π/2, γ) t;
}
