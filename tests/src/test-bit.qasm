OPENQASM 3.0;

include "stdgates.inc"

// bit array declaration with no initialization. Bits will be set to 0.
bit bit_array_1[24];

// bit array declaration + initialization. All bits will be set to 1.
bit bit_array_2[24] = 1;

// bit array declaration + initialization. Bits will be set to the bitmask
// provided in RHS.
bit bit_byte[8] = "00001111";

// bit array declaration with no initialization. Bits will be set to 0.
cbit cbit_array_1[24];

// bit array declaration + initialization. All bits will be set to 1.
cbit cbit_array_2[24] = 1;

// bit array declaration + initialization. Bits will be set to the bitmask
// provided in RHS.
cbit cbit_byte[8] = "00001111";

