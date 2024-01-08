OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// uint number = 15;
uint количество = 15;

// int odin (one) = 1;
int один = 1;

// int dva (two) = 2;
int два = 2;

// int три (three) = 3;
int три = 3;

// int chetyre (four) = 4;
int четыре = 4;

// int pyat (five) = 5;
int пять = 5;

// gate rachmaninov(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate рахманинов(λ, φ, θ) единица {
  U(θ, φ, λ) единица;
}

// gate shostakovich(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate шостакович(λ, φ, θ) единица {
  U(θ, φ, λ) единица;
}
