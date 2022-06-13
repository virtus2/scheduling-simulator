/*
*	Scheduler Algorithm Simulator
*	Github id : virtus2
*
*   lab1_sched.c :
*       - Contains scueduler algorithm test code.
*
*/

#include <aio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <asm/unistd.h>

#include "sched_types.h"



// Processing user input
void ProcessInput()
{
    int i;
    int arrivalTime, serviceTime, ticket;
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        printf("Please enter the Arrival time and Service time of process %c: ", i + 'A');
        scanf("%d %d", &arrivalTime, &serviceTime);
        printf("Please enter the Ticket of the process %c: ", i + 'A');
        scanf("%d", &ticket);
        if (serviceTime == 0 || serviceTime < 0)
        {
            // Invalid input handling
            printf("Service time should not be zero or less than zero. Please enter again.\n");
            i--;
            continue;
        }
        else if (arrivalTime > TOTAL_TIME || arrivalTime < 0)
        {
            // Invalid input handling
            printf("Arrival time should not be greater than %d or less than zero. Please enter again.\n", TOTAL_TIME);
            i--;
            continue;
        }
        else
        {
            // if input is valid, create process
            Process* createdProcess = CreateProcess(i+'A', arrivalTime, serviceTime, ticket);
            // Add created process to array
            processArray[i] = createdProcess;
        }
    }
}

int main(int argc, char *argv[])
{
    int method;
    ProcessInput();
    //printf("Choice scheduling method!: ");
    //scanf("%d", &method);
    printf("\n--------  First In First Out-----------------\n");
    Simulate(FIFO);
    printf("\n--------  Round Robin q=1 -------------------\n");
    Simulate(RR_Q1);
    printf("\n--------  Round Robin q=4 -------------------\n");
    Simulate(RR_Q4);
    printf("\n--------  Shortest Job First ----------------\n");
    Simulate(SJF);
    printf("\n------- Mutli Level Feeback Queue q=1 -------\n");
    Simulate(MLFQ_Q1);
    printf("\n------- Mutli Level Feeback Queue q=4 -------\n");
    Simulate(MLFQ_Q2i);
    printf("\n---------------- LOTTERY --------------------\n");
    Simulate(LOTTERY);
    return 0;
}

