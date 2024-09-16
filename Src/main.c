#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "led.h"

void task1_handler(void); //task1
void task2_handler(void); //task2
void task3_handler(void); //task3
void task4_handler(void); //task4

void init_systick_timer(uint32_t tick_hz);
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_stack);
void init_tasks_stack(void);
void enable_processor_faults(void);
__attribute__((naked)) void switch_sp_to_psp(void);

void task_delay(uint32_t tick_count);

uint8_t current_task = 1; //task1 is running
uint32_t g_tick_count = 0;

typedef struct{
	uint32_t psp_value;
	uint32_t block_count;
	uint8_t current_state;
	void (*task_handler)(void);
}TCB_t;

TCB_t user_tasks[MAX_TASKS];

int main(void)
{
	enable_processor_faults(); //我們要處理stack memory，可能會不小心使用非法指令等等，enable來追蹤這些fault

	init_scheduler_stack(SCHED_STACK_START); //初始化MSP

	init_tasks_stack();

	led_init_all();

	init_systick_timer(TICK_HZ);

	switch_sp_to_psp(); //前面是使用MSP，接下來要使用PSP

	task1_handler();
    /* Loop forever */
	for(;;);
}

void idle_task(void){
	while(1);
}

void task1_handler(void){
	while(1){
		led_on(LED_GREEN);
		task_delay(1000);
		led_off(LED_GREEN);
		task_delay(1000);
	}
}

void task2_handler(void){
	while(1){
		led_on(LED_ORANGE);
		task_delay(500);
		led_off(LED_ORANGE);
		task_delay(500);
	}
}

void task3_handler(void){
	while(1){
		led_on(LED_BLUE);
		task_delay(250);
		led_off(LED_BLUE);
		task_delay(250);
	}
}

void task4_handler(void){
	while(1){
		led_on(LED_RED);
		task_delay(125);
		led_off(LED_RED);
		task_delay(125);
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
	user_tasks[0].current_state = TASK_READY_STATE;
	user_tasks[1].current_state = TASK_READY_STATE;
	user_tasks[2].current_state = TASK_READY_STATE;
	user_tasks[3].current_state = TASK_READY_STATE;
	user_tasks[4].current_state = TASK_READY_STATE;

	user_tasks[0].psp_value = IDLE_STACK_START;
	user_tasks[1].psp_value = T1_STACK_START;
	user_tasks[2].psp_value = T2_STACK_START;
	user_tasks[3].psp_value = T3_STACK_START;
	user_tasks[4].psp_value = T4_STACK_START;

	user_tasks[0].task_handler = idle_task;
	user_tasks[1].task_handler = task1_handler;
	user_tasks[2].task_handler = task2_handler;
	user_tasks[3].task_handler = task3_handler;
	user_tasks[4].task_handler = task4_handler;

	uint32_t *pPSP;

	for(int i = 0; i < MAX_TASKS; i++){
		pPSP = (uint32_t*) user_tasks[i].psp_value;

		pPSP--;
		*pPSP = DUMMY_XPSR; //0X01000000

		pPSP--; //PC
		*pPSP = (uint32_t) user_tasks[i].task_handler;

		pPSP--; //LR
		*pPSP = 0xFFFFFFFD;

		for(int j = 0 ; j < 13 ; j++) //把剩下的R0~R12存0
		{
			pPSP--;
			*pPSP = 0;
		}
		user_tasks[i].psp_value = (uint32_t)pPSP;
	}
}

void enable_processor_faults(void){
	uint32_t *pSHCSR = (uint32_t*)0xE000ED24;

	*pSHCSR |= ( 1 << 16); //mem manage
	*pSHCSR |= ( 1 << 17); //bus fault
	*pSHCSR |= ( 1 << 18); //usage fault
}


uint32_t get_psp_value(void){
	return user_tasks[current_task].psp_value;
}

void save_psp_value(uint32_t current_psp_value){
	user_tasks[current_task].psp_value = current_psp_value;
}

void update_next_task(void){
	int state = TASK_BLOCKED_STATE;

	for(int i= 0 ; i < (MAX_TASKS) ; i++)
	{
		current_task++;
	    current_task %= MAX_TASKS;
		state = user_tasks[current_task].current_state;
		if( (state == TASK_READY_STATE) && (current_task != 0) ) //如果ready，而且不是idle task
			break;
	}

	if(state != TASK_READY_STATE)
		current_task = 0;
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

void schedule(void){
	//Pend the pendsv exception
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= (1 << 28);
}

void task_delay(uint32_t tick_count){
	//disable inturrupt
	INTERRUPT_DISABLE();

	if(current_task){//0是idle task，不要管
		user_tasks[current_task].block_count = g_tick_count + tick_count;
		user_tasks[current_task].current_state = TASK_BLOCKED_STATE;
		schedule();
	}
	//enable inturrupt
	INTERRUPT_ENABLE();
}

__attribute__((naked)) void PendSV_Handler(void){
	/*Save the context of current task*/
	//1.Get current running task's PSP value
	asm volatile("MRS R0, PSP");

	//2. Using that PSP value store SF2(R4~R11)
	//這裡不能用PUSH，因為MSP會被影響
	asm volatile("STMDB R0!,{R4-R11}"); //R0更新PSP value, 儲存R4~R11

	asm volatile("PUSH {LR}"); //因為底下BL的function call會改掉LR，所以先做保存

	//3.Save the current value of PSP
	asm volatile("BL save_psp_value");


	/*Retrieve the context of next task*/
	//1.Decide the next task to run
	asm volatile("BL update_next_task");

	//2.Get its past PSP value
	asm volatile("BL get_psp_value");

	//3.Using that PSP value retrieve SF2(R4~R11)
	asm volatile("LDMIA R0!, {R4-R11}");

	//4.Update PSP and exit
	asm volatile("MSR PSP, R0");

	asm volatile("POP {LR}");

	/*由於是naked function，不會有epilogue，所以要自己寫exception exit*/
	asm volatile("BX LR");
}

void update_global_tick_count(void){
	g_tick_count++;
}

void unblock_tasks(void){
	for(int i = 1; i < MAX_TASKS; i++){
		if(user_tasks[i].current_state != TASK_READY_STATE){
			if(user_tasks[i].block_count == g_tick_count){
				user_tasks[i].current_state = TASK_READY_STATE;
			}
		}
	}
}

void SysTick_Handler(void){
	uint32_t *pICSR = (uint32_t*)0xE000ED04;

	update_global_tick_count();
	unblock_tasks();
	//Pend the pendsv exception
	*pICSR |= (1 << 28);
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

