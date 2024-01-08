OPENQASM 3.0;

// This is a test of UTF8 character encoding in
// OpenQASM3 programs. It has no programmatic
// meaning.

// int ___ü = 10;
int ___ü = 10;

// int ü___ = 10;
int ü___ = 10;

// int ___ü___ = 10;
int ___ü___ = 10;

// int ___bü = 10;
int ___b_ü = 10;

// int b_ü___ = 10;
int b_ü___ = 10;

// int ___b_ü___ = 10;
int ___b_ü___ = 10;

// int ___b_ü_a = 10;
int ___b_ü_a = 10;

// int b_ü_a___ = 10;
int b_ü_a___ = 10;

// int ___b_ü_a___ = 10;
int ___b_ü_a___ = 10;

// int ___b__ü__a___ = 10;
int ___b__ü__a___ = 10;

// int ____b___ü___a____ = 10;
int ___b___ü___a___ = 10;
