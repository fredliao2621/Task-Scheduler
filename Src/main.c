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
void enable_processor_faults(void);
__attribute__((naked)) void switch_sp_to_psp(void);

uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START, T2_STACK_START, T3_STACK_START, T4_STACK_START};
uint32_t task_handlers[MAX_TASKS];
uint8_t current_task = 0; //task1 is running

int main(void)
{
	enable_processor_faults(); //我們要處理stack memory，可能會不小心使用非法指令等等，enable來追蹤這些fault

	init_scheduler_stack(SCHED_STACK_START); //初始化MSP

	task_handlers[0] = (uint32_t)task1_handler; //不同task handler的位址
	task_handlers[1] = (uint32_t)task2_handler;
	task_handlers[2] = (uint32_t)task3_handler;
	task_handlers[3] = (uint32_t)task4_handler;

	init_tasks_stack();

	init_systick_timer(TICK_HZ);

	switch_sp_to_psp(); //前面是使用MSP，接下來要使用PSP

	task1_handler();
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

//更改MSP的值，MSP是special reg，所以要用inline assembly，所以function要做成naked function
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_stack){
	asm volatile("MSR MSP,%0"::"r"(sched_top_stack):);
	//volatile("MSR MSP,R0");
	asm volatile("BX LR");
	//BX(Branch Indirect)把LR的值複製到PC
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

void enable_processor_faults(void){
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= ( 1 << 16); //mem manage
	*pSHCSR |= ( 1 << 17); //bus fault
	*pSHCSR |= ( 1 << 18); //usage fault
}


uint32_t get_psp_value(void){
	return psp_of_tasks[current_task];
}

__attribute__((naked)) void switch_sp_to_psp(void){
	// 1.Initialize the PSP with TASK1 stack start address
	//Get the value of PSP of current task
	asm volatile("PUSH {LR}"); //因為下一個指令要使用BL，會造成原本的LR corrupt，所以先存到stack
	asm volatile("BL get_psp_value"); //不能用B，要用BL(with Link)，因為要回來，回來後current_task會被存在R0
	asm volatile("MSR PSP, R0"); //初始化PSP
	asm volatile("POP {LR}"); //POP原本要回到main的LR

	// 2.Change Change SP to PSP using CONTROL register
	/* CONTROL reg是special reg，要用 MSR，所以要naked*/
	asm volatile("MOV R0,#0x02");
	asm volatile("MSR CONTROL, R0");
	asm volatile("BX LR");
}

void SysTick_Handler(void){

}

void HardFault_Handler(void)
{
	printf("Exception : Hardfault\n");
	while(1);
}


void MemManage_Handler(void)
{
	printf("Exception : MemManage\n");
	while(1);
}

void BusFault_Handler(void)
{
	printf("Exception : BusFault\n");
	while(1);
}

