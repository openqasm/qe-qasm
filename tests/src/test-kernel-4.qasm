OPENQASM 3.0;

uint n = 128;

extern gaussian_1(complex[float[n]] amp, duration d, duration sigma) -> int;

extern gaussian_2(complex[float[n]] amp, duration[15ns] d, duration sigma) -> int;

extern sine(complex[float[n]] amp, duration  d, float[n] freq, angle[n] phs) -> int;

