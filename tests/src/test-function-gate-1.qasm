OPENQASM 3.0;

def function(int i) -> float {
  if (i == 0)
    return pi;

  return pi / 2;
}

gate function_gate(lambda, theta) q {
  U(0.0, lambda, theta) q;
}

qubit $0;

int n = 3;

function_gate(tau, function(n)) $0;
