OPENQASM 3.0;

qubit $q;

stretch a;

delay[a] $q;

duration b = 10ns;

delay[b] $q;

delay[15ns] $q;

