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

int main(void)
{
   status_t st = STATUS_OK;

	st = ConfigureClock();
	if(st!=STATUS_OK)
		Error_Handler(st);

    st = initGPIOA5();
	if(st!=STATUS_OK)
		Error_Handler(st);

    //configure_MCO_pinA8();
    
	st=configure_7_seg_pins();
	if(st!=STATUS_OK)
			Error_Handler(st);

    st = configureUserButton();
	if(st!=STATUS_OK)
		Error_Handler(st);

	st = exti_configure_pa0_pa1();
	if(st!=STATUS_OK)
		Error_Handler(st);

  tim6_init();
  //configure_lpuart_pins();
  //lpuart1_init();

  configure_uart1_pins();
  init_uart1();

	uint8_t i = 0;
  uint8_t debug_timer = 0;
  while (1)
  {
      debug_timer = get_seconds_tim6();

	    send_uart1((uint8_t)(debug_timer +'0'));
	  	  /* if user button pressed, increase i counter */
		  if(GPIOC->IDR & (1<<13)){
       
        toggle_tim6();
			  ++i;
			  /* 7 segment array contains 10 numbers(with 0), if it goes beyond that range, reset the counter */
			  if(i>9)
				  i = 0;
		  }
		  GPIOA->ODR =  segment_numbers[debug_timer];
		  //lpuart1_send_byte(get_seconds_tim6());
		 // delay(2000000);
      send_uart1((uint8_t)'\n');
  }
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
void Error_Handler(status_t st)
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
