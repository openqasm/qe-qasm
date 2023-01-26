OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// uint number = 15;
uint 번호 = 15;

// int one = 1;
int 하나 = 1;

// int two = 2;
int 두의 = 2;

// int three = 3;
int 삼점 = 3;

// int four = 4;
int 네 = 4;

// int five = 5;
int 다섯 = 5;

// gate firstgate(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate 첫번째게이트(λ, φ, θ) 단위 {
  U(θ, φ, λ) 단위;
}

// gate secondgate(lambda, theta, phi) single {
//  U(phi, theta, lambda) single;
// }
gate 두번째게이트(λ, φ, θ) 하나의 {
  U(θ, φ, λ) 하나의;
}

// gate first_gate(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate 첫번째_게이트(λ, φ, θ) 단위 {
  U(θ, φ, λ) 단위;
}

// gate second_gate(lambda, theta, phi) single {
//  U(phi, theta, lambda) single;
// }
gate 두번째_게이트(λ, φ, θ) 하나의 {
  U(θ, φ, λ) 하나의;
}

