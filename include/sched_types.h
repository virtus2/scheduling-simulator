/*
*	Scheduler Algorithm Simulator
*	Github id : virtus2
*
*   sched_types.h :
*       - Contains scueduler algorithm function's declations.
*
*/

#ifndef _SCHED_HEADER_H
#define _SCHED_HEADER_H
 // Maximum length of process array, Default=5
#define PROCESS_COUNT 5
// Total execution time of CPU, Default = 20
#define TOTAL_TIME 20
// Number of Multi Level Feedback Queue 
#define NUMBER_OF_MLFQ 3
enum STATE
{
    STATE_NOT_ARRIVED,
    STATE_ARRIVED,
    STATE_READY,
    STATE_RUNNING,
    STATE_TERMINATED
};
enum METHOD
{
    FIFO,
    RR_Q1,
    RR_Q4,
    SJF,
    LOTTERY,
    MLFQ_Q1,
    MLFQ_Q2i,
};
typedef struct Process
{
    int pid;
    int priority;
    int arrivalTime;
    int serviceTime;
    int elapsedTime;
    int state;
    int ticket;
    struct Process* next;
    struct Process* prev;
}Process;
// Array of Process struct
Process* processArray[PROCESS_COUNT];
// FUNCTIONS
void Simulate();
Process* CreateProcess(int pid, int arrivalTime, int serviceTime, int ticket);
#endif /* SCHED_HEADER_H*/



