OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// uint integer = 15;
uint インテジャー = 15;

// int one = 1;
int ワン = 1;

// int two = 2;
int トゥー = 2;

// int three = 3;
int スリー = 3;

// int four = 4;
int フォー = 4;

// int five = 5;
int ファイブ = 5;

// Using Hiragana characters:
// gate katakana(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate カタカナ(λ, φ, θ) 単位 {
  U(θ, φ, λ) 単位;
}

// Using Katakana characters:
// gate katakana(lambda, theta, phi) unit {
//  U(phi, theta, lambda) unit;
// }
gate 片仮名(λ, φ, θ) ユーニット {
  U(θ, φ, λ) ユーニット;
}
