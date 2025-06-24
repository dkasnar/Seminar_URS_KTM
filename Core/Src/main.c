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
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD_i2c.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRIG_PORT GPIOA
#define TRIG_PIN  GPIO_PIN_1
#define ECHO_PORT GPIOA
#define ECHO_PIN  GPIO_PIN_0
#define PIEZO_PORT GPIOC
#define PIEZO_PIN  GPIO_PIN_7

#define full  2  //definirana udaljenost za puni spremnik



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static int empty = 0;
static uint32_t start = 0;
static uint32_t duration = 0;
volatile int lcdOn = 1;
int lcdPrev = 1;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_IC_capturetureCallback(TIM_HandleTypeDef *htim);
void TIM2_IRQHandler(void);
static int HCSR04_ReadInt(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static int HCSR04_ReadInt(void){
   duration = 0;

   // Generiranje visokog signala 10 µs preko TIM2_CH2
   __HAL_TIM_SET_COUNTER(&htim2, 0);
   __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 10);
   HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);

   //Provjera dolaska signala nazad
   uint32_t limit = HAL_GetTick() + 20 ;
       while (duration == 0 && HAL_GetTick() < limit);
       HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_2);

       if (duration == 0 || duration > 30000){
    	   return -1;
       }
       return (int)duration*(((343.0f*100.0f)/1000000.0f)/2.0f); //izracun udaljenosti
   }

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
        uint32_t capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        if (HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN) == GPIO_PIN_SET) {
            start = capture;
        }
        else {
            uint32_t low = capture;
            if (low >= start){
                duration = low - start;
            }
            else duration = (__HAL_TIM_GET_AUTORELOAD(htim) - start) + low;
        }
    }
}


void TIM2_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim2);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	 char buf[17];
		    int sum;
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
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  	  HAL_TIM_Base_Start(&htim2);
      HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
      HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);

      // LCD inicijalizacija
      HD44780_Init(2);
      HD44780_Backlight();
      HAL_Delay(100);

      //kalibracija za dobivanje praznog spremnika
	  HD44780_Clear();
      HD44780_SetCursor(0,0);
      HD44780_PrintStr("kalibracija...");
      sum = 0;
      for (int i = 0; i <10; i++){
          int d = HCSR04_ReadInt();
            if (d >= 0) {         //uzimamo samo udaljenosti vece od 0
        	  sum = sum + d;
          }
          HAL_Delay(20);
      }
      empty = sum / 10;

      //ispis
      HD44780_Clear();
      HD44780_SetCursor(0,0);
      snprintf(buf, sizeof(buf), "Prazan:%d cm", empty);
      HD44780_PrintStr(buf);
       HAL_Delay(1000);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //paljenje i gašenje lcd ekrana
	  if (lcdOn != lcdPrev) {
	      	    if (lcdOn)  HD44780_Display();
	      	    else  HD44780_NoDisplay();
	      	    lcdPrev = lcdOn;
	      	}
      //Čitanje udaljenosti
	  int distance = HCSR04_ReadInt();

	 if (distance < 0){
		  distance = empty;
	  }
	  //Izracun napunjenosti u cm
	  int level = empty - distance;

	  //Određivanje maksimalne razine spremnika
	  int max_level = empty - full;

	  if (max_level < 1){
		  max_level = 1;
	  }

	  //Pretvorba u postotak
	  int percent;

	  if (distance <= full){
		  percent = 100;
	  }
	    else if (level <= 0){
	    	percent = 0;
	    }
	      else if (level >= max_level){
	    	  percent = 100;
	      }
	        else percent = (level * 100 + max_level/2) / max_level;

	          //Ispis postotka na ekranu
	          HD44780_Clear();
	          HD44780_SetCursor(0,0);      //Postavljanje kursora na prvo mjesto u prvom redu
	          snprintf(buf, sizeof(buf), "%d%%", percent);
	          HD44780_PrintStr(buf);


	          //Graficki prikaz napunjenosti
	          HD44780_SetCursor(0,1);        //Postavljanje kursora na prvo mjesto u drugom redu
	          int filled = percent * 16 / 100;  //koliko kvadrata treba biti ispunjeno

	          for (int i = 0; i < 16; i++){
	        	  if (i < filled){
	        	         buf[i] = 0xFF;         //puni blok
	        	  }
	        	         else buf[i] = ' ';       //prazno polje
	          }

	          buf[16] = '\0';
	          HD44780_PrintStr(buf);
	          HAL_Delay(200);

	       //Paljenje piezo zujalice na 100% napunjenosti
	       if (percent == 100  && lcdOn == 1) {
	        HAL_GPIO_WritePin(PIEZO_PORT, PIEZO_PIN, GPIO_PIN_SET);
	        HAL_Delay(500);

	        HAL_GPIO_WritePin(PIEZO_PORT, PIEZO_PIN, GPIO_PIN_RESET);
	        HAL_Delay(500);
	    }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

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

#ifdef  USE_FULL_ASSERT
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
