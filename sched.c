/*
*	Scheduler Algorithm Simulator
*	Github id : virtus2
*
*   sched.c :
*       - Contains scueduler algorithm function'definition.
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
// Because of MLFQ scheduling, made array of linked list of process
// 0 is used in normal scheduling method
// in MLFQ scheduling method, processList[0]=Q0, [1]=Q1, [2]=Q2... / timeQuantum[0]=q^0, [1]=q^1, [2]=q^2 ...
// priority order: (high) processList[0] > processList[1] > processList[2] > ... (low)
Process* processList[NUMBER_OF_MLFQ];
Process* runningProcess; // current running process
int timeSpent; // spent time after context switch
int timeQuantum[NUMBER_OF_MLFQ]; // time quantum for MLFQ
int totalTickets; // total number of tickets for Lottery
char executionHistory[TOTAL_TIME + 1]; // string to write process execution history
void PushBack(Process* process, int priorityLevel)
{
    // Add process to the back of the list
    if (processList[priorityLevel] == NULL)
    {
        processList[priorityLevel] = process;
        process->prev = NULL;
        process->next = NULL;
        return;
    }
    Process* iterator = processList[priorityLevel];
    while (iterator->next != NULL)
    {
        iterator = iterator->next;
    }
    iterator->next = process;
    process->prev = iterator;
    process->next = NULL;
    process->priority = priorityLevel;
}
Process* Pop(int priorityLevel)
{
    // Remove the first process in list and return
    if (processList[priorityLevel] == NULL)
    {
        return NULL;
    }
    if (processList[priorityLevel]->next == NULL)
    {
        Process* ret = processList[priorityLevel];
        processList[priorityLevel] = NULL;
        return ret;
    }
    else
    {
        Process* ret = NULL;
        processList[priorityLevel]->next->prev = NULL;
        ret = processList[priorityLevel];
        processList[priorityLevel] = ret->next;
        return ret;
    }
}
int FindIndexByPid(int pid, int priorityLevel)
{
    // Find Specific pid process in list and return index of that process
    if (processList[priorityLevel] == NULL)
    {
        printf("process list of %d priority is empty, search failed\n", priorityLevel);
        return -1;
    }
    else
    {
        Process* iterator = processList[priorityLevel];
        int index = 0;
        while (iterator != NULL)
        {
            if (iterator->pid == pid)
            {
                return index;
            }
            iterator = iterator->next;
            index++;
        }
        printf("process pid %d of %d priority not found, search failed\n", pid, priorityLevel);
        return -1;
    }
}
Process* RemoveAt(int index, int priorityLevel)
{
    // Remove specific index process in list and return 
    if (index == 0)
    {
        return Pop(priorityLevel);
    }
    int i;
    Process* iterator = processList[priorityLevel];
    Process* ret = NULL;
    for (i = 0; i < index - 1; i++)
    {
        if (iterator->next == NULL)
        {
            return NULL;
        }
        iterator = iterator->next;
    }
    ret = iterator->next;
    iterator->next = ret->next;
    return ret;
}
Process* Remove(int pid, int priorityLevel)
{
    // Remove specific pid process in list and return
    // find index of process
    int index = FindIndexByPid(pid, priorityLevel);
    if (index >= 0)
    {
        // remove at index
        return RemoveAt(index, priorityLevel);
    }
    else
    {
        /// process not found
        return NULL;
    }
}
void FirstInFirstOut()
{
    if (runningProcess != NULL)
    {
        // Yield to runningProcess to execute
        return;
    }
    else
    {
        // Processes in processQueue are ordered by arrival time
        // so the first element is "first In" process, execute first
        runningProcess = Pop(0);
    }
}
void RoundRobin(int quantum)
{
    if (runningProcess != NULL)
    {
        if (timeSpent >= quantum)
        {
            // timeSpent of runningProcess >= time quantum. Push it back to the list.
            timeSpent = 0;
            PushBack(runningProcess, 0);
            runningProcess = Pop(0);
        }
        else
        {
            // need more time to current runningProcess. Yield to runningProcess
            return;
        }
    }
    else
    {
        runningProcess = Pop(0);
    }
}
void ShortestJobFirst()
{
    int shortestTime = 2147483647;
    Process* iterator = processList[0];
    Process* shortest = NULL;
    // Find Shortest Job in process list
    while (iterator != NULL)
    {
        if (iterator->serviceTime < shortestTime)
        {
            shortestTime = iterator->serviceTime;
            shortest = iterator;
        }
        else if (iterator->serviceTime == shortestTime)
        {
            if (shortest->pid >= iterator->pid)
            {
                shortestTime = iterator->serviceTime;
                shortest = iterator;
            }
        }
        iterator = iterator->next;
    }
    if (runningProcess != NULL)
    {
        // If there are already runningProcess, Yield
        return;
    }
    else
    {
        if (shortest != NULL)
        {
            // Run shortest Job
            runningProcess = Remove(shortest->pid, 0);
        }
    }
}

void ComputeTimeQuantum(int method)
{
    // Calculate time quantum for Multi Level Feedback Queue
    int quantum = 1;
    switch (method)
    {
        case MLFQ_Q1: quantum = 1;
            break;
        case MLFQ_Q2i: quantum = 2;
            break;
        default:
            break;
    }
    int i;
    timeQuantum[0] = 1;
    for (i = 1; i < NUMBER_OF_MLFQ; i++)
    {
        // Qi = q^i
        timeQuantum[i] = timeQuantum[i - 1] * quantum;
    }
}

void MultiLevelFeedbackQueue()
{
    // Priority order 0   >  1  >  2 ...(low number is high priority)
    // Time Quantum   q^0 > q^1 > q^2...
    int i;
    // From high priority queue to low priority 
    // Find highest priority process (front node)
    Process* highest = NULL;
    for (i = 0; i < NUMBER_OF_MLFQ && highest == NULL; i++)
    {
        highest = processList[i];
    }
    if (highest != NULL)
    {
        if (runningProcess != NULL)
        {
            int newPriority = (runningProcess->priority + 1 >= NUMBER_OF_MLFQ) ? NUMBER_OF_MLFQ-1 : runningProcess->priority + 1;
            if (timeSpent >= timeQuantum[runningProcess->priority])
            {
                // If runningProcess's serviceTime > time quantum of Multi Level Feedback Queue
                timeSpent = 0;
                runningProcess->priority = newPriority;
                PushBack(runningProcess, newPriority);
                // Context switch to higher priority process
                runningProcess = Remove(highest->pid, highest->priority);
            }
            else if(timeSpent < timeQuantum[runningProcess->priority])
            {
                // If time slice of current runningProcess is less than timeQuantum, Yield
                return;
            }
        }
        else
        {
            // If there are no runningProcess, execute higher priority process
            runningProcess = Remove(highest->pid, highest->priority);
        }
    }
}
void Lottery()
{
    if (processList[0] == NULL)
    {
        return;
    }
    int counter = 0, totalTickets = 0, winner = 0;
    // Calculate total number of tickets
    Process* iterator = processList[0];
    while (iterator != NULL)
    {
        totalTickets += iterator->ticket;
        iterator = iterator->next;
    }
    // Get a value between 0 and the total # of tickets
    winner = rand() % totalTickets;
    // Loop until the sum of ticket values is greater than winner
    iterator = processList[0];
    while (iterator != NULL)
    {
        counter += iterator->ticket;
        if (counter > winner)
        {
            // found the winner
            break;
        }
        iterator = iterator->next;
    }
    // 'iterator' is the winner: schedule it
    if (runningProcess != NULL)
        PushBack(runningProcess, 0);
    runningProcess = Remove(iterator->pid, 0);
}
void Schedule(int method)
{
    switch (method)
    {
        case FIFO:
            FirstInFirstOut();
            break;
        case RR_Q1:
            RoundRobin(1);
            break;
        case RR_Q4:
            RoundRobin(4);
            break;
        case SJF:
            ShortestJobFirst();
            break;
        case MLFQ_Q1:
            MultiLevelFeedbackQueue();
            break;
        case MLFQ_Q2i:
            MultiLevelFeedbackQueue();
            break;
        case LOTTERY:
            Lottery();
            break;
        default:
            break;
    }
}
Process* CreateProcess(int pid, int arrivalTime, int serviceTime, int ticket)
{
    // Create process 
    Process* newProcess = (Process*)malloc(sizeof(struct Process));
    newProcess->pid = pid;
    newProcess->priority = 0;
    newProcess->arrivalTime = arrivalTime;
    newProcess->serviceTime = serviceTime;
    newProcess->elapsedTime = 0;
    newProcess->state = STATE_NOT_ARRIVED;
    newProcess->next = NULL;
    newProcess->prev = NULL;
    newProcess->ticket = ticket;
    return newProcess;
}
void ProcessArrive(int time)
{
    // If process arrival time is equal to time, Add it to processList
    int i;
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        if (processArray[i]->arrivalTime == time && processArray[i]->state == STATE_NOT_ARRIVED)
        {
            processArray[i]->state = STATE_READY;
            processArray[i]->priority = 0;
            PushBack(processArray[i], 0);
        }
    }
}
void Initialize(int method)
{
    // Initialize variables
    int i;
    Process* iterator = NULL;
    for (i = 0; i < NUMBER_OF_MLFQ; i++)
    {
        iterator = processList[i];
        while (iterator != NULL)
        {
            iterator->next = NULL;
            iterator->prev = NULL;
            iterator = iterator->next;
        }
        processList[0] = NULL;
        timeQuantum[i] = 0;
    }
    for (i = 0; i < TOTAL_TIME; i++)
    {
        executionHistory[i] = '\0';
    }
    runningProcess = NULL;
    timeSpent = 0;
    // Reset processes for re-using inputs
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        processArray[i]->state = STATE_NOT_ARRIVED;
        processArray[i]->elapsedTime = 0;
        processArray[i]->next = NULL;
        processArray[i]->prev = NULL;
    }

    // if scheduling method is MLFQ, Calculate the time quantum of each MLFQ
    switch (method)
    {
        case LOTTERY:
            break;
        case MLFQ_Q1:
        case MLFQ_Q2i:
            ComputeTimeQuantum(method);
            break;
        default:
            break;
    }
    // Random for lottery scheduling
    srand(time(NULL));
}
void PrintHistory()
{
    int i;
    printf("TIME: %d to %d\n", 0, TOTAL_TIME);
    printf("A ");
    for (i = 0; i < TOTAL_TIME; i++)
    {
        if (executionHistory[i] == 'A') printf("A");
        else printf("_");
    }
    printf("\n");
    printf("B ");
    for (i = 0; i < TOTAL_TIME; i++)
    {
        if (executionHistory[i] == 'B') printf("B");
        else printf("_");
    }
    printf("\n");
    printf("C ");
    for (i = 0; i < TOTAL_TIME; i++)
    {
        if (executionHistory[i] == 'C') printf("C");
        else printf("_");
    }
    printf("\n");
    printf("D ");
    for (i = 0; i < TOTAL_TIME; i++)
    {
        if (executionHistory[i] == 'D') printf("D");
        else printf("_");
    }
    printf("\n");
    printf("E ");
    for (i = 0; i < TOTAL_TIME; i++)
    {
        if (executionHistory[i] == 'E') printf("E");
        else printf("_");
    }
    printf("\n");
}
void Simulate(int method)
{
    // Simulate process scheduling 
    int time = 0;
    Initialize(method);
    while (time <= TOTAL_TIME)
    {
        ProcessArrive(time); // When process arrives at time, add it to list
        Schedule(method); // Using scheduling methods, Pick one process to execute 
        if (runningProcess != NULL)
        {
            // process execution simulate
            runningProcess->state = STATE_RUNNING;
            runningProcess->elapsedTime++;
            timeSpent++;
            executionHistory[time] = runningProcess->pid;
            if (runningProcess->elapsedTime >= runningProcess->serviceTime)
            {
                // process's work complete
                runningProcess->state = STATE_TERMINATED;
                runningProcess = NULL;
                timeSpent = 0;
            }
        }
        else
        {
            // There are no processes to run, CPU goes idle
            executionHistory[time] = '.';
        }
        time++;
    }
    PrintHistory();
}
