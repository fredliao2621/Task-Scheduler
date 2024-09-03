#include <stdint.h>
#include <stdio.h>

void task1_handler(void); //task1
void task2_handler(void); //task2
void task3_handler(void); //task3
void task4_handler(void); //task4

void init_systick_timer(uint32_t tick_hz);

/* Some stack memory calculations */
#define SIZE_TASK_STACK    1024U
#define SIZE_SCHED_STACK   1024U

#define SRAM_START         0x20000000U
#define SIZE_SRAM          ((128) * (1024))
#define SRAM_END           ((SRAM_START) + (SIZE_SRAM))

#define T1_STACK_START     SRAM_END
#define T2_STACK_START     ((SRAM_END) - (1 * SIZE_TASK_STACK))
#define T3_STACK_START     ((SRAM_END) - (2 * SIZE_TASK_STACK))
#define T4_STACK_START     ((SRAM_END) - (3 * SIZE_TASK_STACK))
#define SCHED_STACK_START  ((SRAM_END) - (4 * SIZE_TASK_STACK))

#define TICK_HZ             1000U
#define HSI_CLK             16000000U
#define SYSTICK_TIM_CLK     HSI_CLK

int main(void)
{
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

void SysTick_Handler(void){

}
