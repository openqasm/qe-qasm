OPENQASM 3.0;

uint i = 128U;

rotr(i, 8);

int s = 8;

rotr(i, s);

uint[128] mpi = 128U;

rotr(mpi, 8);

rotr(mpi, s);

bit[32] b = "10101010101010101010101010101010";

rotr(b, 8);

int r = rotr(i, s);

uint u = rotr(mpi, 8);

r = rotr(mpi, s);

r = rotr(b, 8);

