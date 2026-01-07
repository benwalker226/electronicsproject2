#ifndef LCR_MEASURE_H
#define LCR_MEASURE_H

#include <stdint.h>
#include <STDbool.h>

typedef struct {
  float voltage;
  float current;
  float highRange; // Feedback resistor state
} LCR_Measurement_t;

void LCR_PerformMeasurement(LCR_Measurement_t *result);

#endif
