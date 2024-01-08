OPENQASM 3.0;

include "stdgates.inc"

// fixed-point uninitialized.
fixed fa[24, 16];

// fixed-point initialized to PI.
fixed fb[24, 12] = pi;

// fixed-point initialized to PI / 2.
fixed fc[24, 12] = pi / 2;

// fixed-point initialized to a constant floating-point.
fixed fd[32, 12] = 0.3152347;

// fixed-point initialization to TAU.
fixed fe[24, 12] = tau;

// fixed-point initialization to euler_gamma.
fixed ff[24, 12] = euler_gamma;

// fixed-point initialization to TAU * 2.
fixed fg[24, 12] = tau * 2;

// fixed-point initialization to euler_gamma + 3.15.
fixed fh[24, 12] = euler_gamma + 3.15;
