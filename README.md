# Round-Robin Task Scheduler on STM32F407G-DISC1

## Overview
This project implements a round-robin task scheduler on the STM32F407G-DISC1 board. The scheduler uses the SysTick and PendSV interrupts to handle multiple tasks in a circular fashion, ensuring each task gets an equal time slice for execution. The scheduler manages context switching between tasks by saving and restoring the processor's state.

The system includes four user tasks, each controlling an LED with different blink patterns, and an idle task that runs when no other tasks are ready.

## Features
- **Round-robin scheduling**: Tasks are scheduled in a circular order with equal time slices.
- **Task control blocks (TCBs)**: Each task has its own stack and state information stored in a TCB.
- **Context switching**: The scheduler saves and restores task context (registers, stack pointer, etc.) during each switch.
- **Idle task**: The scheduler runs an idle task when no other tasks are in the ready state.
- **LED control**: Tasks toggle different LEDs with different delay patterns, demonstrating multitasking.

## File Structure
- `main.c`, `main.h`: Contains the main logic for the scheduler and task management.
- `led.c`, `led.h`: Used to control the onboard LEDs.

## Components

### 1. Stack Setup
Each task has its own stack in the memory, initialized in `init_tasks_stack()`. The main stack pointer (MSP) is used for interrupt handling, while each task uses the process stack pointer (PSP) for task execution.

### 2. Task Control Block (TCB)
Each task is represented by a `TCB_t` structure, which holds:
- `psp_value`: Task's PSP value.
- `block_count`: Counter to manage blocked state delays.
- `current_state`: The state of the task (ready or blocked).
- `task_handler`: Pointer to the task function.

### 3. SysTick Timer
Configured to trigger at 1 kHz (1 ms intervals). It calls the `SysTick_Handler`, which increments the global tick count and handles task unblocking.

### 4. PendSV Handler
Manages the context switching between tasks. When a task needs to be switched, it saves the current task's context (R4-R11) and loads the next task's context.

## Tasks
- **Task 1** (Green LED): Toggles every 1000ms ON, 1000ms OFF.
- **Task 2** (Orange LED): Toggles every 500ms ON, 500ms OFF.
- **Task 3** (Blue LED): Toggles every 250ms ON, 250ms OFF.
- **Task 4** (Red LED): Toggles every 125ms ON, 125ms OFF.
- **Idle Task**: Runs when no other tasks are ready. It doesn't perform any action.

## Key Functions

### 1. `main()`
Initializes the SysTick timer, sets up stacks for tasks, and starts the scheduler.

### 2. `init_tasks_stack()`
Initializes the stack for each task, setting up the PSP, LR, PC, and dummy values for other registers (R0-R12).

### 3. `task_delay(uint32_t tick_count)`
Blocks the current task for a specified number of ticks, switching to the next ready task.

### 4. `PendSV_Handler()`
A naked function that performs the context switch by saving the current task’s context and loading the next task’s context.

### 5. `SysTick_Handler()`
Updates the global tick count and unblocks tasks whose block count has expired. It also triggers the PendSV exception for context switching.

## Usage

### 1. Build and Upload
Use STM32CubeIDE to compile the project and upload it to the STM32F407G-DISC1.

### 2. LED Behavior
After uploading the firmware, the LEDs on the board will blink at different intervals, demonstrating the round-robin scheduling of the tasks.

## Configuration

### Memory Layout
Task stacks and scheduler stack are set up in SRAM, with each task assigned 1KB of stack space.

### Interrupt Handling
The `SysTick_Handler` and `PendSV_Handler` are responsible for scheduling and context switching.
