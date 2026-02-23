/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "seven_segment.h"


int main(void)
{
   status_t st = STATUS_OK;
  /* USER CODE BEGIN 1 */
	st = ConfigureClock();
	if(st!=STATUS_OK)
		return st;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/



  /* USER CODE BEGIN Init */
    st = initGPIOA5();
	if(st!=STATUS_OK)
		return st;

	st=configure_7_seg_pins();
	if(st!=STATUS_OK)
			return st;
    st = configureUserButton();
	if(st!=STATUS_OK)
		return st;
	st = configureInterrupt();
	if(st!=STATUS_OK)
		return st;
//  /* USER CODE END Init */



  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */


  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	uint8_t i = 0;
  while (1)
  {
    /* USER CODE END WHILE */


	  	  /* if user button pressed, increase i counter */
		  if(GPIOC->IDR & (1<<13)){
			  ++i;
			  /* 7 segment array contains 9 numbers, if it goes beyond that range, reset the counter */
			  if(i>8)
				  i = 0;
		  }
		  GPIOA->ODR =  segment_numbers[i];

		  delay(2000000);


	//  }

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */

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
