OPENQASM 3.0;

uint u = 15;

int i = 31;

uint[128] mpu = "1234567890987654321";

int[128] mpi = "9876543210123456789";

bit[32] b = "01010101010101010101010101010101";

uint p = popcount(u);

p = popcount(i);

p = popcount(mpu);

p = popcount(mpi);

p = popcount(b);

