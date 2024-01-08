OPENQASM 3.0;

uint i = 128U;

rotl(i, 8);

int s = 8;

rotl(i, s);

uint[128] mpi = 128U;

rotl(mpi, 8);

rotl(mpi, s);

bit[32] b = "01010101010101010101010101010101";

rotl(b, 8);

int r = rotl(i, s);

uint u = rotl(mpi, 8);

r = rotl(mpi, s);

r = rotl(b, 8);
