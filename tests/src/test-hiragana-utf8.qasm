OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// uint integer = 15;
uint 整数 = 15;

// int A = 1;
int あ = 1;

// int E = 2;
int え = 2;

// int I = 3;
int い = 3;

// int U = 4;
int う = 4;

// int O (oh) = 5;
int お = 5;

// Using Hiragana characters:
// gate hiragana(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate ひらがな(λ, φ, θ) 単位 {
  U(θ, φ, λ) 単位;
}

// Using Katakana characters:
// gate hiragana(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate 平仮名(λ, φ, θ) ユーニット {
  U(θ, φ, λ) ユーニット;
}
