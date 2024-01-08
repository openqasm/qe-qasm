OPENQASM 3.0;

uint size = 32;

extern gaussian(complex[float[size]] amp, duration d, duration sigma) -> int;

float[size] f = 35.35;

duration ds = 15ns;

duration ss = 150us;

int r = gaussian(f, ds, ss);
