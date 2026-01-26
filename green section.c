/*
 * ui.c
 *
 *  Created on: 05 Jan 2026
 *      Author: Vivek
 */
// ui.c

// Requires linker flag for float printf:  -u _printf_float
// Phase is shown in degrees
// Units are aligned to the right side with minimalised labelling

#include "ui.h"
#include "DAJP_F303K8_Driver.h"
#include <stdio.h>
#include "main.h"

static int DisplayMode = 0;
// DisplayMode = 0, Impedance and Phase
// DisplayMode = 1, Resistance
// DisplayMode = 2, Capacitance or Inductance

// Code for the LCD_Print2 function
static void LCD_Print_2(const char *l0, const char *l1) // Local function so that we dont have to repeat these processes every time
{
    LCR_LCD_Clear();
    LCR_LCD_GoToXY(0, 0);
    LCR_LCD_WriteString((char*)l0, 16);
    LCR_LCD_GoToXY(0, 1);
    LCR_LCD_WriteString((char*)l1, 16);
}

static void UI_ShowMeasurement(const LCR_Measurement_t *m)
{
    char line0[17];
    char line1[17];

// ---------------------------------------------------------------------------Mode 0: Z magnitude + phase (degrees)---------------------

    if (DisplayMode == 0)
    {
        float phase_deg = m->z_phase_rad * 57.2958f; // radian to degree conversion

        // Here labelling and units take up 6 units with the remaining 10 units for the measured value
        if (m->z_mag < 1000.0f) // Checks the magnitude of R{Z}
        {
            snprintf(line0, sizeof(line0), "Z:%10.2f ohm", m->z_mag);
        }
        else // If the value is greater than 1000, then this applies the magnitude prefix
        {
            snprintf(line0, sizeof(line0), "Z:%9.2f kOhm", m->z_mag / 1000.0f);
        }

        snprintf(line1, sizeof(line1), "Ph:%9.1f deg", phase_deg);

        // We dont need filler for line 1 because DisplayMode 0 uses both line
        LCD_Print_2(line0, line1);

        return;
    }

// --------------------------------------------------------------------------------------------Mode 1: Resistance-----------------------

    if (DisplayMode == 1)
    {
        if (m->z_real < 1000.0f) // Conversion for kilo prefix
        {
            snprintf(line0, sizeof(line0), "R:%10.2f ohm", m->z_real);
        }
        else
        {
            snprintf(line0, sizeof(line0), "R:%9.2f kOhm", m->z_real / 1000.0f);
        }

        snprintf(line1, sizeof(line1), "                "); // Prints the resistance on line 0 and clears line 1
        LCD_Print_2(line0, line1);

        return;

    }

// -----------------------------------------------------------------------------------Mode 2: Capacitance / Inductance------------------

	if (DisplayMode == 2)
	{
		if (m->derived_C > 0.0f) // Code for capacitance if theres a reading
		{
			float C_uF = m->derived_C * 1e6f; // Conversions for micro and nano prefixes
			float C_nF = m->derived_C * 1e9f;

			if (C_uF >= 1.0f)
			{
				snprintf(line0, sizeof(line0), "C:%11.2f uF", C_uF);
			}
			else
			{
				snprintf(line0, sizeof(line0), "C:%11.2f nF", C_nF);
			}

			snprintf(line1, sizeof(line1), "                "); // Prints the capacitance on line 0 and clears line 1
			LCD_Print_2(line0, line1);

		}

		else if (m->derived_L > 0.0f) // Code for Inductance if theres a reading
		{
			float L_mH = m->derived_L * 1e3f; // Conversions for milli and micro prefixes
			float L_uH = m->derived_L * 1e6f;

			if (L_mH >= 1.0f)
			{
				snprintf(line0, sizeof(line0), "L:%11.2f mH", L_mH);
			}
			else
			{
				snprintf(line0, sizeof(line0), "L:%11.2f uH", L_uH);
			}

			snprintf(line1, sizeof(line1), "                "); // Prints the inductance on line 0 and clears line 1
			LCD_Print_2(line0, line1);
		}

		else
		{
			LCD_Print_2("C/L: ---", "                "); // Error case if neither capacitance or inductance have no readings
		}

		return;
	}

//--------------------------------------------------------------------------------------------------------------------------------------

	else
	{
		LCD_Print_2("Mode error", "                "); // Error case for DisplayMode
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------

void UI_GreenSectionLoop(const LCR_Measurement_t *m)
{
    UI_ShowMeasurement(m); // Shows the current measurement on the LCD

    while (1)
    {
    	// This cycles through the DisplayModes
        if (LCR_Switch_GetState(0) == 0)
        {
            while (LCR_Switch_GetState(0) == 0) {}
            HAL_Delay(200);

            DisplayMode = (DisplayMode + 1) % 3; // This limits the DisplayModes to a maximum of 2
            UI_ShowMeasurement(m);
        }

        // This sends the LCR meter back to measuring
        if (LCR_Switch_GetState(1) == 0)
        {
            while (LCR_Switch_GetState(1) == 0) {}
            HAL_Delay(200);
            return;
        }

        HAL_Delay(50);
    }
}























