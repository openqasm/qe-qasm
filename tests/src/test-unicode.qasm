OPENQASM 3.0;

bit Γ;

bit Φ;

int δ = 3;

uint ζ = 4U;

angle[32] Σ = 3.141592;

angle[32] ω = 151.3;

gate unicode(λ, θ) β {
  U(π, λ, θ) β;
}

gate Z q {
  U(π, π / 2, π / 4) q;
}

gate noangles β, υ {
  Z β;
  Z υ;
}
