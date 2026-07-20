#include "scheduler.h"

static uint8_t task_counter;

uint32_t g_run_time;
task_parameters  tasks_array[MAX_TASKS_COUNT];

void scheduler_init(void){
    for(uint8_t i =0; i < MAX_TASKS_COUNT;++i){
        tasks_array[i].callback_args = 0;
        tasks_array[i].last_time_called = 0;
        tasks_array[i].period = 0;
        tasks_array[i].scheduler_task_callback = 0;
    }
 task_counter = 0;
 g_run_time = get_ms();
}




int8_t scheduler_add_task(scheduler_task callback, void* args, uint32_t period){
    if(callback==NULL || period == 0U || task_counter >= MAX_TASKS_COUNT)
        return -1;

    
    tasks_array[task_counter].period = period;
    tasks_array[task_counter].scheduler_task_callback = callback;
    tasks_array[task_counter].callback_args = args;
    tasks_array[task_counter].last_time_called = get_ms();

   return ++task_counter;
 
}
void scheduler_run(void){
    g_run_time = get_ms();
    for(uint8_t i = 0; i < task_counter;++i){

        if( (uint32_t)(g_run_time - tasks_array[i].last_time_called) >= tasks_array[i].period){

        tasks_array[i].scheduler_task_callback(tasks_array[i].callback_args);
        tasks_array[i].last_time_called = get_ms();
        }

    }
}