#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stm32g431xx.h>
#include <stddef.h>
#include "sys_tick.h"

#define MAX_TASKS_COUNT 25

typedef void (*scheduler_task)(void* arg);


typedef struct 
{
    uint32_t last_time_called;
    uint32_t period;
    scheduler_task scheduler_task_callback;
    void* callback_args;
}task_parameters;


void scheduler_init(void);
int8_t scheduler_add_task(scheduler_task callback, void* args, uint32_t period);
void scheduler_run(void);

#endif /* SCHEDULER_H_ */