/*
 * reg_bits_check.h
 *
 *  Created on: March 8, 2026
 *      Author: romanyarmak
 */
#ifndef INC_REG_BITS_CHECK_H
#define INC_REG_BITS_CHECK_H

#include <stdint.h>
#include <stm32g431xx.h>
#include "status.h" 

#ifndef READY_TIMEOUT
#define READY_TIMEOUT 0X5000U
#endif

/*
 * Bacause of using static functions (with inline) for embedded efficiency, its realization can be done in .c file,
 * it would be make them invisible to another .c files, so it would be poitless
*/

static inline status_t isBitSet(volatile const uint32_t *reg, const uint32_t mask, const  uint32_t timeout){

	uint32_t temp = timeout;
	if(temp == 0U){
		return WRONG_TIMEOUT;
	}

	if (mask == 0U) {
    return WRONG_MASK;
	}
	/* as soon as REG bits  to msk's bits are NOT 0, loop breaks */
	/* REG  0b0101001|1|110 /
	 *         &(AND)
	 * mask 0b0000000|1|000/
	 * result is     |1|
	 */
	while(((*reg) & mask) ==0u ){
		--temp;
		if(temp == 0U){
			return BIT_FLAG_IS_ZERO;
		}

	}
	return STATUS_OK;
}

static inline status_t isValueSet(volatile const uint32_t *reg, const uint32_t mask, const  uint32_t timeout, const uint32_t value){

	uint32_t temp = timeout;
	if(temp == 0U){
	return WRONG_TIMEOUT;
	}

	if (mask == 0U) {
    return WRONG_MASK;
	}
				
	/* check if bits after mask equal to gave value */
	/* REG  0b0101001|10|10 /
	 *         &(AND)
	 * mask 0b0000000|11|00/
	 * result is     |10|00
	 */

	while(((*reg) & mask) != value ){
#ifdef DEBUG_MODE
		uint32_t debug = ((*reg) & mask);
#endif
		--temp;
		if(temp == 0U){
			return MSK_REG_VAL_DOESNT_MATCH;
		}

	}
	return STATUS_OK;
}

static inline status_t isBitZero(volatile const uint32_t *reg, const uint32_t mask, const uint32_t timeout){
	/* as soon as REG bits  to msk's bits are NOT 1, loop breaks */
	/* REG  0b0101001|0|110 /
	 *         &(AND)
	 * mask 0b0000000|1|000/
	 * result is     |0|
	 */
	uint32_t temp = timeout;
	if(temp == 0U){
	return WRONG_TIMEOUT;
	}
	if (mask == 0U) {
    return WRONG_MASK;
	}
	while( ((*reg) & mask)!=0u ){
#ifdef DEBUG_MODE
		uint32_t debug = ((*reg) & mask);
#endif
		--temp;
		if(temp == 0U){
			return BIT_ISNT_ZERO;
		}
	}

	return STATUS_OK;
}




#endif /* INC_REG_BITS_CHECK_H */