OPENQASM 3.0;

gate CX a, b { ctrl @ U(π, 0, π) a, b; }
