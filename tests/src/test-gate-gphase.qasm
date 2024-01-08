OPENQASM 3.0;

gate u3(θ, φ, λ) q {
  gphase(-(φ+λ)/2);
  U(θ, φ, λ) q;
}
