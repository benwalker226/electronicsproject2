#include "lcr_measure.h"
#include "DAJP_F303K8_Driver.h"
#include <math.h>

#define ADC_MAX            4095.0f
#define VREF               3.3f
#define VBIAS              (VREF * 0.5f)    // your waveform is centered at mid-rail
#define CURRENT_SENSE_R    100.0f           // R3 = 100 ohms

// These depend on your analogue gain settings (op-amp feedback range).
// Set them to measured/calibrated values later. For now keep = 1.
#define V_GAIN_LOW         1.0f
#define V_GAIN_HIGH        1.0f
#define I_GAIN_LOW         1.0f
#define I_GAIN_HIGH        1.0f

// coherent sampling settings (match your DAC table size)
#define WAVEFORM_SAMPLES   64
#define MEAS_CYCLES        8               // take multiple cycles for stable phase
#define N_SAMPLES          (WAVEFORM_SAMPLES * MEAS_CYCLES)

static uint32_t adcBuf[N_SAMPLES * 2];      // interleaved CH1,CH2,CH1,CH2...

static inline float adc_to_volts(uint32_t code)
{
    return ((float)code / ADC_MAX) * VREF;
}

void LCR_PerformMeasurement(LCR_Measurement_t *result, float freq_hz)
{
    // 1) Read switch position (range/feedback resistor state)
    result->highRange = (LCR_RangeSwitch_Get() != 0);

    // 2) "Send current through component"
    //    (In your design the DAC excitation is already running continuously.
    //     If you later gate it, you’d enable it here.)

    // Small settle time so op-amp/bias stabilises before capture
    // (keep short—measurement should be fast)
    LCR_MicroDelay(2000);

    // 3) Take multiple synchronous samples so phase can be calculated
    // Sampling rate must be coherent: Fs = freq_hz * WAVEFORM_SAMPLES
    uint32_t sample_rate = (uint32_t)(freq_hz * (float)WAVEFORM_SAMPLES);

    // Ensure ADC sampling system is initialised elsewhere once (recommended),
    // but calling init repeatedly is still safe if your driver supports it.
    // Best practice: call LCR_ADC_Sample_Init() once in main() after init.
    LCR_ADC_GetSamples(adcBuf, N_SAMPLES, sample_rate);

    // 4) Lock-in / single-bin DFT at fundamental (coherent with 64 samples/cycle)
    float V_re = 0, V_im = 0;
    float I_re = 0, I_im = 0;

    float V_sq = 0, I_sq = 0;

    for (int n = 0; n < N_SAMPLES; n++)
    {
        // interleaved: [V0, I0, V1, I1, ...]
        float v = adc_to_volts(adcBuf[2*n + 0]) - VBIAS;  // remove DC bias
        float i_sense = adc_to_volts(adcBuf[2*n + 1]) - VBIAS;

        // apply gain correction for current range (placeholder until calibrated)
        float v_gain = result->highRange ? V_GAIN_HIGH : V_GAIN_LOW;
        float i_gain = result->highRange ? I_GAIN_HIGH : I_GAIN_LOW;

        v /= v_gain;
        i_sense /= i_gain;

        // Current through DUT from sense resistor
        float i = i_sense / CURRENT_SENSE_R;

        // RMS (time-domain) for display/debug
        V_sq += v * v;
        I_sq += i * i;

        // reference sin/cos for coherent fundamental
        int k = n % WAVEFORM_SAMPLES;
        float ang = 2.0f * (float)M_PI * ((float)k / (float)WAVEFORM_SAMPLES);
        float c = cosf(ang);
        float s = sinf(ang);

        V_re += v * c;
        V_im += v * s;
        I_re += i * c;
        I_im += i * s;
    }

    // Convert accumulated sums to phasors (scale for single-bin DFT)
    float scale = 2.0f / (float)N_SAMPLES;
    V_re *= scale; V_im *= scale;
    I_re *= scale; I_im *= scale;

    result->v_rms = sqrtf(V_sq / (float)N_SAMPLES);
    result->i_rms = sqrtf(I_sq / (float)N_SAMPLES);

    // 5) Compute complex impedance Z = V / I
    float denom = (I_re*I_re + I_im*I_im);
    if (denom < 1e-12f)
    {
        // Avoid divide by zero
        result->z_real = 0;
        result->z_imag = 0;
        result->z_mag = 0;
        result->z_phase_rad = 0;
        result->derived_C = 0;
        result->derived_L = 0;
        return;
    }

    // (a+jb)/(c+jd) = ((ac+bd)+j(bc-ad))/(c^2+d^2)
    float Z_re = (V_re*I_re + V_im*I_im) / denom;
    float Z_im = (V_im*I_re - V_re*I_im) / denom;

    result->z_real = Z_re;
    result->z_imag = Z_im;
    result->z_mag = sqrtf(Z_re*Z_re + Z_im*Z_im);
    result->z_phase_rad = atan2f(Z_im, Z_re);

    // 6) Derive C or L from reactance
    result->derived_C = 0.0f;
    result->derived_L = 0.0f;

    if (freq_hz > 0.0f)
    {
        float w = 2.0f * (float)M_PI * freq_hz;

        if (Z_im < 0.0f)
        {
            // capacitive: Xc = -1/(wC) => C = -1/(w*X)
            result->derived_C = -1.0f / (w * Z_im);
        }
        else if (Z_im > 0.0f)
        {
            // inductive: Xl = wL => L = X/w
            result->derived_L = Z_im / w;
        }
    }
}
