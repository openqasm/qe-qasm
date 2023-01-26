// stdopenpulse.inc - Standard OpenPulse header file
// Defines standard operations on Frames.

defcalgrammar "openpulse";

def set_phase(frame f, angle ph) {
  f.phase = ph;
}

def get_phase(frame f) -> angle {
  return f.phase;
}

def shift_phase(frame f, angle ph) {
  f.phase = ph;
}

def set_frequency(frame f, float freq) {
  f.frequency = freq;
}

def get_frequency(frame f) -> float {
  return f.frequency;
}

def shift_frequency(frame f, float freq) {
  f.frequency = freq;
}

