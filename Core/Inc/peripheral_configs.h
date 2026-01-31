/*
 * peripheral_configs.h
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */
#include <stdint.h>
#include <stm32g431xx.h>
#include "status.h"
#ifndef INC_PERIPHERAL_CONFIGS_H_
#define INC_PERIPHERAL_CONFIGS_H_

/**
  * @brief  The application entry point.
  * @retval int
  */


 static inline status_t isBitSet(volatile const uint32_t *reg, const uint32_t mask, const  uint32_t timeout){

	uint32_t temp = timeout;
	if(temp<=0){
		return WRONG_TIMEOUT;
			}
	/* as soon as REG bits  to msk's bits are NOT 0, loop breaks */
	/* REG  0b0101001|1|110 /
	 *         &(AND)
	 * mask 0b0000000|1|000/
	 * result is     |1|
	 */
	while(((*reg) & mask) ==0u ){
		--temp;
		if(temp<=0){
			return BIT_FLAG_IS_ZERO;
		}

	}
	return STATUS_OK;
}

static inline status_t isValueSet(volatile const uint32_t *reg, const uint32_t mask, const  uint32_t timeout, const uint32_t value){

	uint32_t temp = timeout;
	if(temp<=0){
			return WRONG_TIMEOUT;
				}
	/* check if bits after mask equal to gave value */
	/* REG  0b0101001|10|10 /
	 *         &(AND)
	 * mask 0b0000000|11|00/
	 * result is     |10|
	 */

	while(((*reg) & mask) != value ){
#ifdef DEBUG_MODE
		uint32_t debug = ((*reg) & mask);
#endif
		--temp;
		if(temp<=0){
			return MSK_REG_VAL_DOESNT_MATCH;
		}

	}
	return STATUS_OK;
}

static inline int isBitZero(volatile const uint32_t *reg, const uint32_t mask, const uint32_t timeout){
	/* as soon as REG bits  to msk's bits are NOT 1, loop breaks */
	/* REG  0b0101001|0|110 /
	 *         &(AND)
	 * mask 0b0000000|1|000/
	 * result is     |0|
	 */
	uint32_t temp = timeout;
	if(temp<=0){
			return WRONG_TIMEOUT;
				}
	while( ((*reg) & mask)!=0u ){
#ifdef DEBUG_MODE
		uint32_t debug = ((*reg) & mask);
#endif
		--temp;
		if(temp<=0){
			return BIT_ISNT_ZERO;
		}
	}

	return STATUS_OK;
}

int configureUserButton();

status_t ConfigureClock();

int initGPIOA5();

int delay(const uint32_t timer);







#endif /* INC_PERIPHERAL_CONFIGS_H_ */
