/*
 * status.h
 *
 *  Created on: Jan 31, 2026
 *      Author: romanyarmak
 */

#ifndef INC_STATUS_H_
#define INC_STATUS_H_

 typedef enum{
	STATUS_OK = 0x00U,
	WRONG_TIMEOUT = 0x01U,
	CLOCK_ERROR = 0x02U,
	BIT_FLAG_IS_ZERO = 0x03U,
	BIT_ISNT_ZERO = 0x04U,
	MSK_REG_VAL_DOESNT_MATCH = 0x05U

}status_t;

#endif /* INC_STATUS_H_ */
