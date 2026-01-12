#ifndef LCR_MEASURE_H
#define LCR_MEASURE_H

#include <stdint.h>
#include <STDbool.h>

typedef struct {
  bool highRange; // feedback resistor state (range switch)
  float v_rms; // Vrms across DUT (AC)
  float i_rms; // Irms through DUT (AC)
  float z_mag; // |Z| (ohms)
  float z_phase_rad; // phase of Z (rad)
  float z_real; // Re(Z) ohms 
  float z_imag; // Im(Z) ohms
  float derived_C; // if not capacitive 
  float derived_L; // if not inductive
  // float voltage;
  // float current;
  // float highRange; // Feedback resistor state
} LCR_Measurement_t;
// Perform multiplem sampels so phase/ complez impedance is posisble 
// (assuming DAC excitation is already running)
void LCR_PerformMeasurement(LCR_Measurement_t *result, float freq_hz);

// void LCR_PerformMeasurement(LCR_Measurement_t *result);

#endif
