OPENQASM 3.0;

qubit $0;
qubit $1;

defcal delay[100ms] $0 { }

defcal delay[1000ns] $1 { }

defcal "testgrammar" delay[100ms] $0 { }

defcal "testgrammar" delay[1000ns] $1 { }

