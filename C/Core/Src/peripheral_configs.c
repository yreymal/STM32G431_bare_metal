/*
 * peripheral_configs.c
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */
#include "peripheral_configs.h"
#include "reg_bits_check.h"



status_t configure_MCO_pinA8(){
	if(!(RCC->AHB2ENR & RCC_AHB2ENR_GPIOAEN)){
       RCC->AHB2ENR|= RCC_AHB2ENR_GPIOAEN_Msk;
	}
	GPIOA->MODER &= ~GPIO_MODER_MODE8_Msk;
	/* 10: Alternate function mode */
	GPIOA->MODER|= (2UL<<GPIO_MODER_MODE8_Pos);
	/* 0: Output push-pull */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT0_Msk;

	GPIOA->OSPEEDR &= ~ GPIO_OSPEEDR_OSPEED8_Msk;
	/* 11: Very high speed */
	GPIOA->OSPEEDR|= ( 3UL << GPIO_OSPEEDR_OSPEED8_Pos);

	/* 00: No pull-up, pull-down*/
	GPIOA->PUPDR &= ~ GPIO_PUPDR_PUPD8_Msk;
    /* 0000: AF0 */
	GPIOA->AFR[1] &= ~  GPIO_AFRH_AFSEL8_Msk;

	RCC->CFGR &= ~  RCC_CFGR_MCOPRE_Msk;
	/* 011: MCO is divided by 16 */
	RCC->CFGR|= (RCC_CFGR_MCOPRE_DIV16);

	RCC->CFGR &= ~ RCC_CFGR_MCOSEL_Msk;
	/* 0001: SYSCLK system clock selected */
	/* 0101: Main PLL clock selected */
	RCC->CFGR|= (1UL << RCC_CFGR_MCOSEL_Pos);

	return STATUS_OK;
}

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

status_t ConfigureClock(void){

	/* function return check */
	 status_t st = STATUS_OK;
	//----------------------------------------------------------------------------
    // 1. Enable HSI and switch temporarily to HSI
    //----------------------------------------------------------------------------

	/* for safety enable and switch temporarily to HSI */
	RCC->CR|=(1UL<<RCC_CR_HSION_Pos);
	/* wait until the HSI rdy bit is set, or return with error status */
	st = isBitSet(&RCC->CR, RCC_CR_HSIRDY_Msk, READY_TIMEOUT);
	if(st!=STATUS_OK)
		return st;

	/* switch clock source from  HSI16, 
	* clear SW bits, then write bits for HSI in CFGR register
	*/
	RCC->CFGR = (RCC->CFGR & (~RCC_CFGR_SW_Msk)) | (RCC_CFGR_SW_HSI);
	/* wait until the HSI clock set, or return with error status*/
	st = isValueSet(&RCC->CFGR, RCC_CFGR_SWS_Msk, READY_TIMEOUT, RCC_CFGR_SWS_HSI);
	if(st!=STATUS_OK){
		return st;
	}

	///----------------------------------------------------------------------------
    // 2. Enable HSE
    //----------------------------------------------------------------------------
	/* HSE crystal mode, NOT bypass */
	RCC->CR &= ~RCC_CR_HSEBYP;
	/* turning ON HSE */
	RCC->CR|=(1UL<<RCC_CR_HSEON_Pos);

	

	/* wait until the HSE rdy bit is set, or return with error status */
	st = isBitSet(&RCC->CR, RCC_CR_HSERDY_Msk, READY_TIMEOUT);
	if(st!=STATUS_OK)
		return st;
	//----------------------------------------------------------------------------
    // 3. Disable PLL before reconfiguration
    //----------------------------------------------------------------------------
	/* turning off PLL before set its configuration */
	RCC->CR&=(~RCC_CR_PLLON_Msk);

	/* wait until the PLL rdy bit is set to 0, or return with error status */
	st = isBitZero(&RCC->CR,RCC_CR_PLLRDY_Msk, READY_TIMEOUT);
	if(st!=STATUS_OK){
		return st;
	}
    //----------------------------------------------------------------------------
    // 4. Configure FLASH latency before increasing SYSCLK
    //----------------------------------------------------------------------------

	/* configure Flash waiting states before configurating PLL */
	/* clean latency bits */
	FLASH->ACR&=(~FLASH_ACR_LATENCY_Msk);
	/* setting 2 WS for FLASH */
	/* 3 WC if CLCK<= 120 MHz, we use 100 MHz */
	FLASH->ACR|=(FLASH_ACR_LATENCY_3WS<<FLASH_ACR_LATENCY_Pos);

	 //----------------------------------------------------------------------------
    // 5. Set bus prescalers
    //----------------------------------------------------------------------------

	/* clear peripheria prescale bits */
	/* PPR1 - APB1, PPR2 - APB2, HPRE - AHB */
	RCC->CFGR&=~(RCC_CFGR_PPRE1_Msk|RCC_CFGR_PPRE2_Msk|RCC_CFGR_HPRE_Msk);
	/* set clock buses deviders */

    RCC->CFGR|= (APB1_PRESCALER_REG /* APB1 pres = 1 */
				|APB2_PRESCALER_REG /* APB2 pres = 1 */
				|AHB_PRESCALER_REG); /* HPRE pres = 1 */
	
    //----------------------------------------------------------------------------
    // 6. Configure PLL: HSE=24 MHz -> SYSCLK=100 MHz
    //
    // PLLM = 3   => 24 / 3 = 8 MHz
    // PLLN = 25  => 8 * 25 = 200 MHz
    // PLLR = 2   => 200 / 2 = 100 MHz
    //----------------------------------------------------------------------------

	/* clear multi-bits fields before setting */
	RCC->PLLCFGR&=~(RCC_PLLCFGR_PLLSRC_Msk|RCC_PLLCFGR_PLLN_Msk|RCC_PLLCFGR_PLLM_Msk|RCC_PLLCFGR_PLLR_Msk);
	RCC->PLLCFGR|=((0b11u<<RCC_PLLCFGR_PLLSRC_HSE_Pos) //set HSE as PLL source
				|(PLLM_DIV_REG_VAL<<RCC_PLLCFGR_PLLM_Pos)         //set PLLM=3
				|(PLLN_MUL<<RCC_PLLCFGR_PLLN_Pos)           //set PLLN=16
				|(PLLR_DIV_REG_VAL<<RCC_PLLCFGR_PLLR_Pos)		       //set PLLR=2
				|(RCC_PLLCFGR_PLLREN_Msk));        //enabling PLL
	//----------------------------------------------------------------------------
    // 7. Enable PLL
    //----------------------------------------------------------------------------
    /*turning ON PLL and wait for PLLRDY flag set as 1, or return with error status*/
	RCC->CR|=(1UL<<RCC_CR_PLLON_Pos);
	st = isBitSet(&RCC->CR,RCC_CR_PLLRDY_Msk, READY_TIMEOUT);
	if(st!=STATUS_OK)
		return st;
	//----------------------------------------------------------------------------
    // 8. Switch SYSCLK to PLL
    //----------------------------------------------------------------------------	
	/* clear multi-bits fields before setting */
	RCC->CFGR&=(~RCC_CFGR_SW_Msk);

	/* switch SYSCL to PLL */
	RCC->CFGR|=(RCC_CFGR_SW_PLL<<RCC_CFGR_SW_Pos);

	/* wait until the SWS set as PLL, or return with error status */
	st = isValueSet(&RCC->CFGR,RCC_CFGR_SWS_Msk, READY_TIMEOUT, RCC_CFGR_SWS_PLL);
	if(st!=STATUS_OK)
		return st;
	//----------------------------------------------------------------------------
    // 9. Disable HSI, no longer needed
    //----------------------------------------------------------------------------

	/*turning off HSI */
    RCC->CR &= ~RCC_CR_HSION_Msk;

	/* wait until HSI status set as turned off, or return with error status */
	st = isBitZero(&RCC->CR, RCC_CR_HSIRDY_Msk, READY_TIMEOUT);
	if(st!=STATUS_OK)
		return st;
return st;
}

status_t initGPIOA5(void)
{
    status_t st = STATUS_OK;

    /* enable clock for GPIOA */
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN_Msk;

    st = isBitSet(&RCC->AHB2ENR, RCC_AHB2ENR_GPIOAEN_Msk, READY_TIMEOUT);
    if (st != STATUS_OK) {
        return st;
    }

    /* PA5 -> general purpose output mode: 01 */
    GPIOA->MODER &= ~GPIO_MODER_MODE5_Msk;
    GPIOA->MODER |=  (1UL << GPIO_MODER_MODE5_Pos);

    st = isValueSet(&GPIOA->MODER, GPIO_MODER_MODE5_Msk, READY_TIMEOUT, (1UL << GPIO_MODER_MODE5_Pos));
    if (st != STATUS_OK) {
        return st;
    }

    /* PA5 -> push-pull: 0 */
    GPIOA->OTYPER &= ~GPIO_OTYPER_OT5_Msk;

    st = isBitZero(&GPIOA->OTYPER, GPIO_OTYPER_OT5_Msk, READY_TIMEOUT);
    if (st != STATUS_OK) {
        return st;
    }

    /* PA5 -> no pull-up / no pull-down: 00 */
    GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_Msk;

    st = isValueSet(&GPIOA->PUPDR, GPIO_PUPDR_PUPD5_Msk, READY_TIMEOUT, 0U);
    if (st != STATUS_OK) {
        return st;
    }

    /* PA5 -> low speed: 00 */
    GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5_Msk;

    st = isValueSet(&GPIOA->OSPEEDR, GPIO_OSPEEDR_OSPEED5_Msk, READY_TIMEOUT, 0U);
    if (st != STATUS_OK) {
        return st;
    }

    return STATUS_OK;
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
