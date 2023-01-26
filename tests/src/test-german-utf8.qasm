OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// uint bear = 15;
uint Bär = 15;

// int one = 1;
int einz = 1;

// int two = 2;
int zwei = 2;

// int three = 3;
int drei = 3;

// int four = 4;
int vier = 4;

// int five = 5;
int fünf = 5;

// gate resilient(lambda, theta, phi) ø {
//  U(phi, theta, lambda) ø;
// }
gate unverwüstlich(λ, φ, θ) ø {
  U(θ, φ, λ) ø;
}

// gate ferocious(lambda, theta, phi) ö {
//  U(phi, theta, lambda) ö;
// }
gate gründlich(λ, φ, θ) ö {
  U(θ, φ, λ) ö;
}

