/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "DAJP_F303K8_Driver.h"
#include "lcr_measure.h" // include measure function
#include <stdio.h>
#include <math.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define WAVEFORM_SAMPLES 64
#define DAC_RATE 100000 // 100 kHz sample rate
#define PI 3.14159265358979323846
// The frequency of the signal generated:
// 100,000 Hz sample rate / 64 samples per wave = 1562.5 Hz
#define TEST_FREQ 1562.5

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t sine_wave[WAVEFORM_SAMPLES];
uint32_t V_data[WAVEFORM_SAMPLES * 3];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

void MX_GPIO_Init(void);
void Initialize_Sine_Wave(void);
void LCR_Meter_Run(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * @brief Simple function to fill the sine_wave buffer for the DAC.
 */
void Initialize_Sine_Wave(void) {
    const uint32_t max_val = 4095;
    const uint32_t mid_val = max_val / 2;

    #ifndef M_PI
    #define M_PI 3.14159265358979323846
    #endif

    for (int i = 0; i < WAVEFORM_SAMPLES; i++) {
        float angle = (float)i / WAVEFORM_SAMPLES * 2.0 * M_PI;
        // Generate sine wave centred at mid_val
        sine_wave[i] = (uint32_t)(mid_val + mid_val * sin(angle));
    }
}

/**
 * @brief Main execution loop for the LCR meter's logic.
 */
void LCR_Meter_Run(void) {
    // 1. Initial Screen
    LCR_LCD_Clear();
    LCR_LCD_GoToXY(0, 0);
    LCR_LCD_WriteString("Button Test Mode", 16);
    LCR_LCD_GoToXY(0, 1);
    LCR_LCD_WriteString("Press buttons...", 16);
    HAL_Delay(1500);
    LCR_LCD_Clear();

    while (1) {
        // --- CHECK BUTTON 0 (PF0) ---
        LCR_LCD_GoToXY(0, 0); // Top Line
        if (LCR_Switch_GetState(0) == 0) {
            // Button is pulled LOW when pressed
            LCR_LCD_WriteString("Btn 0: PRESSED ", 16);
        } else {
            // Button is pulled HIGH (1) when released
            LCR_LCD_WriteString("Btn 0: Open    ", 16);
        }

        // --- CHECK BUTTON 1 (PB1) ---
        LCR_LCD_GoToXY(0, 1); // Bottom Line
        if (LCR_Switch_GetState(1) == 0) {
            LCR_LCD_WriteString("Btn 1: PRESSED ", 16);
        } else {
            LCR_LCD_WriteString("Btn 1: Open    ", 16);
        }

        // Small delay to prevent screen flickering
        HAL_Delay(100);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	/* USER CODE BEGIN 1 */

	  /* USER CODE END 1 */

	  /* MCU Configuration--------------------------------------------------------*/

	  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	  HAL_Init();

	  /* USER CODE BEGIN Init */

	  /* USER CODE END Init */

	  /* Configure the system clock */
	  SystemClock_Config();

	  /* USER CODE BEGIN SysInit */

	  /* USER CODE END SysInit */

	  /* Initialize all configured peripherals */

	  // Custom Driver Initialization Start

	  // Initialize the sine wave data
	  Initialize_Sine_Wave();

	  // Initialize user inputs (Switches, ADC analog pins PA0, PA1, PA8)
	  LCR_Init_Inputs();

	  // Initialize the LCD screen (uses PA9, PA10, PA12, PB0, PB6, PB7)
	  LCR_LCD_Init();

	  // Start the Function Generator (PA4) to output the AC excitation signal
	  LCR_FuncGen_Init(sine_wave, WAVEFORM_SAMPLES, TEST_FREQ);

	  /* USER CODE BEGIN 2 */

	  /* USER CODE END 2 */

	  /* Infinite loop */
	  /* USER CODE BEGIN WHILE */
	  LCR_Meter_Run();

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

