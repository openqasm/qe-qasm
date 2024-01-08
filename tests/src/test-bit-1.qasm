OPENQASM 3.0;

// bit array declaration with no initialization. Bits will be set to 0.
bit[24] bit_array_1;

// bit array declaration + initialization. All bits will be set to 1.
bit[24] bit_array_2 = 1;

// bit array declaration + initialization. Bits will be set to the bitmask
// provided in RHS.
bit[8] bit_byte = "00001111";

// bit array declaration with no initialization. Bits will be set to 0.
cbit[24] cbit_array_1;

// bit array declaration + initialization. All bits will be set to 1.
cbit[24] cbit_array_2 = 1;

// bit array declaration + initialization. Bits will be set to the bitmask
// provided in RHS.
cbit[8] cbit_byte = "00001111";
