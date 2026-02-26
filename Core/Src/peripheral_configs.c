/*
 * peripheral_configs.c
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */
#include "peripheral_configs.h"



int configureUserButton(){

	/* user button is connected to port C, pin 13 */
	/* enabling clock on port C */
	RCC->AHB2ENR|=RCC_AHB2ENR_GPIOCEN_Msk;
	/* configure pin 13 as input without pulling up/down,
	 * because there is HW pull down resistor for the button on board
	 * |00| - Input mode */
	GPIOC->MODER&=(~GPIO_MODER_MODE13_Msk);
	/* it's important to turn off pull up/down, because of external HW pull down
	 * 00: No pull-up, pull-down */
	GPIOC->PUPDR&=(~GPIO_PUPDR_PUPD13_Msk);
	return 0;
}

status_t ConfigureClock(){
	/* function return check */
	 status_t st = STATUS_OK;
	//HSI
	/* for safety reasons assure that we use HSI */
	RCC->CR|=(1<<RCC_CR_HSION_Pos);
	/* wait until the rdy bit is set */
	st = isBitSet(&RCC->CR, RCC_CR_HSIRDY_Msk, 0x5000);
	if(st!=STATUS_OK)
		return st;
	/* switch system clock to work from  HSI16 */
	RCC->CFGR = (RCC->CFGR & (~RCC_CFGR_SW_Msk)) | (RCC_CFGR_SW_HSI);
	/* wait until the clock's switch status is HSI*/
	st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, 0x1000, RCC_CFGR_SWS_HSI);
	if(st!=STATUS_OK)
		return st;
	//HSE
	/* turning ON HSE */
	RCC->CR|=(1<<RCC_CR_HSEON_Pos);
	/* wait until the rdy bit is set */
	st = isBitSet(&RCC->CR, RCC_CR_HSERDY_Msk, 0x5000);
	if(st!=STATUS_OK)
		return st;
	//PLL
	/* turning off PLL before tailor it */
	RCC->CR&=(~RCC_CR_PLLON_Msk);
	st = isBitZero(&RCC->CR,RCC_CR_PLLRDY_Msk,0x5000);
	if(st!=STATUS_OK)
		return st;
	/* configure Flash waiting states before configurating PLL */
	/* clean latency bits */
	FLASH->ACR&=(~FLASH_ACR_LATENCY_Msk);
	/* setting 2 WS for FLASH */
	/* 2 WC if CLCK<= 90 MHz, we use 64 MHz */
	FLASH->ACR|=(FLASH_ACR_LATENCY_2WS<<FLASH_ACR_LATENCY_Pos);

	/* clear peripheria prescale bits */
	/* PPR1 - APB1, PPR2 - APB2, HPRE - AHB */
	RCC->CFGR&=~(RCC_CFGR_PPRE1_Msk|RCC_CFGR_PPRE2_Msk|RCC_CFGR_HPRE_Msk);

	/*configurate PLL */
	/* HSE = 24 MHz, purpose - SYSCLC = 64 MHz
	 * SYSCLC = Vc/PLLR, PLLR =2
	 * PLLM = 3. =>24/3 = 8 MHz
	 * PLLN = 16, 16 * 8 = 12GPIO_MODER_MODE5_Pos8MHz
	 * 128/2(PLLR) = 64MHz */
	RCC->PLLCFGR&=~(RCC_PLLCFGR_PLLSRC_Pos|RCC_PLLCFGR_PLLN_Pos|RCC_PLLCFGR_PLLM_Pos|RCC_PLLCFGR_PLLR_Pos);
	RCC->PLLCFGR|=((0b11u<<RCC_PLLCFGR_PLLSRC_HSE_Pos) //set HSE as PLL source
				|(0b10u<<RCC_PLLCFGR_PLLM_Pos)         //set PLLM=3
				|(16u<<RCC_PLLCFGR_PLLN_Pos)           //set PLLN=16
				|(0u<<RCC_PLLCFGR_PLLR_Pos)		       //set PLLR=2
				|(1u<<RCC_PLLCFGR_PLLREN_Pos));        //enabling PLL

   /*turning ON PLL and wait for PLLRDY flag set as 1*/
	RCC->CR|=(1<<RCC_CR_PLLON_Pos);
	st = isBitSet(&RCC->CR,RCC_CR_PLLRDY_Msk,0x5000);
	if(st!=STATUS_OK)
		return st;
	/* switch SYSCL to PLL */
	RCC->CFGR&=(~RCC_CFGR_SW_Msk);
	RCC->CFGR|=(RCC_CFGR_SW_PLL<<RCC_CFGR_SW_Pos);
	st = isValueSet(&RCC->CFGR,RCC_CFGR_SWS_Msk, 0x5000, RCC_CFGR_SWS_PLL);
	if(st!=STATUS_OK)
		return st;
	/*turning off HSI */
	RCC->CR&=(~RCC_CR_HSION_Msk);
	st = isBitZero(&RCC->CR,RCC_CR_HSIRDY_Msk,0x5000);
	if(st!=STATUS_OK)
		return st;
return st;
}

int initGPIOA5(){
	/* start clock for port A */
	RCC->AHB2ENR|=(1ul<<RCC_AHB2ENR_GPIOAEN_Pos);
	/* clean bits before setting  */
	GPIOA->MODER&=~GPIO_MODER_MODE5_Msk;
	/* |01| - General purpose output mode */
	GPIOA->MODER|=(1u<<GPIO_MODER_MODE5_Pos);

	GPIOA->OTYPER&=~GPIO_OTYPER_OT5_Msk;
	/* |0| - Output push-pull */
	GPIOA->OTYPER&=(~GPIO_OTYPER_OT5_Pos);
	/* No pull-up, pull-down */
	GPIOA->PUPDR&=~GPIO_PUPDR_PUPD5_Msk;
	/* |0| - Low speed */
	GPIOA->OSPEEDR&=~GPIO_OSPEEDR_OSPEED5_Msk;
	return 0;
}

int delay(const uint32_t timer){
	uint32_t temp = timer;
	if(temp>0){
		while(temp--){
			__asm volatile ("nop");

		}
	}
	else{
		return WRONG_TIMEOUT;
	}
	return 0;
}
