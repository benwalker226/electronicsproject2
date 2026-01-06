#include "lcr_measure.h"
#include "DAJP_F303K8_Driver.h"
#include "stm32f3xx_hal.h"

// ADC reference and resoltion
#define ADC_MAX      4095.0f
#define VREF         3.3f

// resistor R3
#define CURRENT_SENSE_R   100.0f  // R3 = 100 ohms

// ADC channels
#define ADC_VOLTAGE_CH    0   // PA0
#define ADC_CURRENT_CH    1   // PA1

// Feedback switch pin index
#define FEEDBACK_SWITCH

void LCR_DoMeasurement(LCR_Measurement_t * result){

  uint32_t RawVoltage;
  uint32_t rawCurrent;

  /* Read feedback resistor switch */
  result->highRange = (LCR_Switch_GetState(FEEDBACK_SWITCH) !=0);

  /* Allow signal to settle */
  HAL_Delay(5);

  /* Read voltage ADC */
  rawVotlage = LCR_getADC_read(ADC_VOTLAGE_CH);

  /* Read current ADC */
  rawCurrent = LCR_GetADC_Read(ADC_CURRENT_CH);

  /* then convert to actual units */
  float voltage = (rawVoltage / ADC_MAX) *VREF;
  float currentSenseVoltage = (rawCurrent / ADC_MAX) * VREF;

  /* calculate current */
  float current = currentSenseVoltage / CURRENT_SENSE_R

  result->voltage = voltage;
  result->current = current;
  
}
