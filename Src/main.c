#include <stdint.h>
#include <stdio.h>
#include "main.h"

void task1_handler(void); //task1
void task2_handler(void); //task2
void task3_handler(void); //task3
void task4_handler(void); //task4

void init_systick_timer(uint32_t tick_hz);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_stack);
void init_tasks_stack(void);

uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START, T2_STACK_START, T3_STACK_START, T4_STACK_START};
uint32_t task_handlers[MAX_TASKS];

int main(void)
{
	init_scheduler_stack(SCHED_STACK_START); //初始化MSP

	task_handlers[0] = (uint32_t)task1_handler; //不同task handler的位址
	task_handlers[1] = (uint32_t)task2_handler;
	task_handlers[2] = (uint32_t)task3_handler;
	task_handlers[3] = (uint32_t)task4_handler;

	init_tasks_stack();

	init_systick_timer(TICK_HZ);
    /* Loop forever */
	for(;;);
}

void task1_handler(void){
	while(1){
		printf("This is task1\n");
	}
}

void task2_handler(void){
	while(1){
		printf("This is task2\n");
	}
}

void task3_handler(void){
	while(1){
		printf("This is task3\n");
	}
}

void task4_handler(void){
	while(1){
		printf("This is task4\n");
	}
}

void init_systick_timer(uint32_t tick_hz){
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	uint32_t *pSCSR = (uint32_t*)0xE000E010;
	uint32_t count_value = (SYSTICK_TIM_CLK / tick_hz) - 1;

	//Clear the value of SVR
	*pSRVR &= ~(0x00FFFFFF);

	//Load the value of SVR
	*pSRVR |= count_value;

	//Do some setting
	*pSCSR |= ( 1 << 1); //Enables SysTick exception request:
	*pSCSR |= ( 1 << 2);  //Indicates the clock source, processor clock source

	//Enable the systick
	*pSCSR |= ( 1 << 0); //enables the counter
}

__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_stack){
	asm volatile("MSR MSP,%0"::"r"(sched_top_stack):);
	//volatile("MSR MSP,R0"); 也可以用這個指令
	asm volatile("BX LR");
}

void init_tasks_stack(void){
	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS; i++){
		pPSP = (uint32_t*)psp_of_tasks[i];

		pPSP--;
		*pPSP = DUMMY_XPSR; //0X01000000

		pPSP--; //PC
		*pPSP = task_handlers[i];

		pPSP--; //LR
		*pPSP = 0xFFFFFFFD;

		for(int j = 0 ; j < 13 ; j++) //把剩下的R0~R12存0
		{
			pPSP--;
			*pPSP = 0;
		}
		psp_of_tasks[i] = (uint32_t)pPSP;
	}
}

void SysTick_Handler(void){

}
