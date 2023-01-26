OPENQASM 3.0;

qubit $0;

gate rz(theta) q { }

float[128] f1 = 1.5707963267949;

float[128] f2 = 7.03508951031555;

rz(f1) $0;

rz(f2) $0;

