OPENQASM 3.0;

qubit $0;
qubit $1;

duration da = 350μs;

duration db = 500ms;

defcal delay[da] $0 { }

defcal delay[db] $1 { }

defcal "testgrammar" delay[da] $0 { }

defcal "testgrammar" delay[db] $1 { }

