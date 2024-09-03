#include <stdint.h>

void task1_handler(void); //task1
void task2_handler(void); //task2
void task3_handler(void); //task3
void task4_handler(void); //task4

int main(void)
{
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
