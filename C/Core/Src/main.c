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

static volatile display_refresh_par dis_par;
static volatile display_refresh_par display_cfg;
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

  init_portc();

  SysTick_1_Init_ms(SYSCLK_HZ);
  scheduler_init();

  configure_MCO_pinA8();

  TIM8_CH1_CH2_PC6_PC7_OutputCompare_Init();
  
	uint8_t i = 0;
  uint8_t digit_number= 0;
  uint8_t debug_timer = 0;

 
 //dis_par.digit_position = 0;
 dis_par.digit_number = 0;

segments_init(&dis_par);
  //scheduler_add_task(send_uart1, 2000, (uint8_t)(debug_timer +'0') );
 

  
    
     scheduler_add_task(store_seconds_tim6, &dis_par, 400);
     scheduler_add_task(calculate_segments_for_digit, &dis_par, 200 );
     scheduler_add_task(show_digit_on_display, &dis_par, 4);
  //scheduler_add_task(get_seconds_tim6, 500, 0);

  //show_digit_on_display(dis_par.digit_number, dis_par.digit_position);

  //scheduler_add_task(test_display, 300, &i);
  while (1)
  {
   
   scheduler_run();
   //test_display(&i);
    //  debug_timer = get_seconds_tim6();
    //  digit_number = get_digit();
  
	 //   send_uart1((uint8_t)(dis_par.digit_number +'0'));
	  	  /* if user button pressed, increase i counter */
		//  if(GPIOC->IDR & (1<<13)){
   // store_seconds_tim6(&dis_par);
   // show_digit_on_display(&dis_par);
  // toggle_tim6();
     // }
			//   /* 7 segment array contains 10 numbers(with 0), if it goes beyond that range, reset the counter */
			//   if(i>9)
			// 	  i = 0;
		  
		 // GPIOA->ODR =  segment_numbers[8];
		  //lpuart1_send_byte(get_seconds_tim6());
		  
     //show_digit_on_display(debug_timer);
     //show_digit_on_dis_pos(2, 2);
     //delay(2000000);
   //   send_uart1((uint8_t)'\n');
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
