/**
 * Family Name:     
 * Given Name:      
 * Section:         
 * Student Number:  
 * CSE Login:       
 *
 * Description: First-come-first-serve (quad-core) CPU scheduling simulator.
 */

/* WARNING: This code is used only for you to debug your own FCFS scheduler. You should NOT use this as a template for the remaining schedulers. Otherwise, your codes may not pass plagiarism tests. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "sch-helpers.h"    /* include header file of all helper functions */

/* Declare some global variables and structs to be used by FCFS scheduler */
process processes[MAX_PROCESSES+1];   // a large structure array to hold all processes read from data file 
int numberOfProcesses;              // total number of processes 
int nextProcess;
int clock;
int waitingTime;
int contextSwitch;
int cpuUtilizedTime;
//int completionTime; 


process_queue readyQueue;           //Create queue for all processes that are read
process_queue waitngQueue;          //Create queue for all processes that are waiting for I/O

process *CPUS[NUMBER_OF_PROCESSORS];    //CPUs
process *tempPreReady[MAX_PROCESSES];       //Creates a temprary queue that is used before the ready state
int tempQSize;


/** Initialization functions **/

void resetVar(void)                 // Will reset all the variables
{
    int i;
    for (i=0;i<NUMBER_OF_PROCESSORS;i++) {
        CPUS[i] = NULL;
    }
    numberOfProcesses = 0;
    nextProcess = 0;
    clock = 0;
    waitingTime = 0;
    contextSwitch = 0;
    cpuUtilizedTime = 0;
    //completionTime = 0;
    tempQSize = 0;

    initializeProcessQueue(&readyQueue);
    initializeProcessQueue(&waitngQueue);
}


/** FCFS scheduler simulation functions **/

int comparePIDs (const void *a, const void *b)
{
    process *one = *((process**) a);
    process *two = *((process**) b);
    if (one->pid < two->pid) return -1;
    if (one->pid > two->pid) return 1;


   /* if ((*one).pid == (*two).pid)
    {
        error_duplicate_pid((*one).pid);
    }
*/
    assert(0); /* case should never happen, o/w ambiguity exists */
    return 0;
    
}


int findRunningProcesses(void)
{
    int running = 0;
    int i;
    for(i = 0; i < NUMBER_OF_PROCESSORS; i++)   //Iterate through all the cpus to find which ones have processes
    {
        if(CPUS[i] != NULL)     // if the CPU is not at a null state it has a processs
        {
            running++;      //hence increment the # of total Processes running
        }
    }
    return running;
}


int findTotalInProcess(void)
{
    int result;
    result = numberOfProcesses - nextProcess;
    return result;
}

/* enqueue newly arriving processes in the ready queue */
void addNewProcess(void)
{
    while (nextProcess < numberOfProcesses && processes[nextProcess].arrivalTime <= clock)
    {
        tempPreReady[tempQSize++] = &processes[nextProcess++];  //Incoming process placed in tempPreReady queue to be later used by ready queue
    }

}

/* simulates the CPU scheduler, fetching and dequeuing the next scheduled
   process from the ready queue.  it then returns a pointer to this process,
   or NULL if no suitable next process exists. */
process *getNextProcess (void)
{
    if (readyQueue.size == 0)
    {
        return NULL;    //Because there is nothing in the ready queue at the moment 
    }
    process *getNext = readyQueue.front->data;  //get the element in the front of the ready queue (FIFO)
    dequeueProcess(&readyQueue);        //using library function to remove process from the front of the list 
    return getNext;

}


/* move any waiting processes that are finished their I/O bursts to ready */
void waitingToReadyProcess(void)
{
    int i;
    int s = waitngQueue.size;
    for(i = 0; i < s; i++)
    {
        process *getNext = waitngQueue.front->data;
        dequeueProcess(&waitngQueue);

         assert(getNext->bursts[getNext->currentBurst].step <=
               getNext->bursts[getNext->currentBurst].length);

        //Check if no. of steps completed is equal to time steps required for the burst
        if(getNext->bursts[getNext->currentBurst].step == 
            getNext->bursts[getNext->currentBurst].length)
        {
            getNext->currentBurst++;    //Move to the next I/O burst by updating the waiting state.
            tempPreReady[tempQSize++] = getNext;    //Increase temp Q size by one and add the process that finished waiting
        }
        else {
            enqueueProcess(&waitngQueue, getNext);
        }
    }
}


/* move ready processes into free cpus according to scheduling algorithm */
void readyToRunningProcess(void)
{
    qsort(tempPreReady, tempQSize, sizeof(process*), comparePIDs);      //sorts the temporary queue of processes based on their PID
    int j;
    for(j = 0; j < tempQSize; j++)
    {
        enqueueProcess(&readyQueue, tempPreReady[j]);   //add the sorted array into the ready queue using helper class function
    }
    tempQSize = 0;  //Reset the tempPreReady size to zero

    int k;
    for(k = 0; k < NUMBER_OF_PROCESSORS; k++)
    {
        if(CPUS[k] == NULL)     //if any of the CPUS are available then it will be assigned a process
        {
            CPUS[k] = getNextProcess(); //Assign the next process for the CPU
        }
    }
}

/* move any running processes that have finished their CPU burst to waiting,
   and terminate those that have finished their last CPU burst. */
void runningToWaitingProcess(void)
{
    int i;
    for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
    {
        if(CPUS[i] != NULL)     //CPU is running a process
        {
            

            //To check if the CPU burst has completed
            if(CPUS[i]->bursts[CPUS[i]->currentBurst].step ==
                CPUS[i]->bursts[CPUS[i]->currentBurst].length)
            {
                CPUS[i]->currentBurst++; //The processes next I/O burst
                
            

            if (CPUS[i]->currentBurst < CPUS[i]->numberOfBursts) {
                enqueueProcess(&waitngQueue, CPUS[i]);  //If the running process has finished their CPU burst, move them back to 
                                                        //the waiting queue for I/O bursts. 
            }

            else
            {
                CPUS[i]->endTime = clock;       //Terminates current process by setting its time to the clock time
            }

                CPUS[i] = NULL;
            }   
        }
    }
}

/* increment each waiting process' current I/O burst's progress */
void updateWaiting(void)
{
    int i;
    int s = waitngQueue.size;
//To update the waiting state
        for (i = 0; i < s; i++)
        {
            process *getNext = waitngQueue.front->data;
            dequeueProcess(&waitngQueue);
            getNext->bursts[getNext->currentBurst].step++;
            enqueueProcess(&waitngQueue, getNext);
        }
}

/* increment waiting time for each process in the ready queue */
void updateReady(void) {
    int i;
    for (i=0;i<readyQueue.size;i++) {
        process *front = readyQueue.front->data; /* get process at front */
        dequeueProcess(&readyQueue);             /* dequeue it */
        front->waitingTime++;                    /* increment waiting time */
        enqueueProcess(&readyQueue, front);      /* enqueue it again */
    }
}

/* update the progress for all currently executing processes */
void updateRunningProcesses(void) {
    int i;
    for (i=0;i<NUMBER_OF_PROCESSORS;i++) {
        if (CPUS[i] != NULL) {
            /* increment the current (CPU) burst's step (progress) */
            CPUS[i]->bursts[CPUS[i]->currentBurst].step++;
        }
    }
}

int main(void) {
    int status = 0;
    int totalTurnaround=0;


    resetVar();     //Resets all global variables 



    while (status=readProcess(&processes[numberOfProcesses]))  
    {
         if(status==1)
         {
            numberOfProcesses ++;
         }

         if(numberOfProcesses > MAX_PROCESSES)
         {
            break;
         }

    }   

     if (numberOfProcesses == 0) 
     {
        fprintf(stderr, "Error: no processes.\n");
        return -1;
    } 

    else if (numberOfProcesses > MAX_PROCESSES) {
        fprintf(stderr, "Error: too many processes specified in input; "
                        "they cannot number more than %d.\n", MAX_PROCESSES);
        return -1;
    }

// it reads pid, arrival_time, bursts to one element of the above struct array

    qsort(processes, numberOfProcesses, sizeof(process), compareByArrival);

    while(1)
    {
        addNewProcess();
        runningToWaitingProcess();
        readyToRunningProcess();
        waitingToReadyProcess();

        updateWaiting();
        updateReady();
        updateRunningProcesses();
    

        

        cpuUtilizedTime += findRunningProcesses();

        //if there are no more processes to run
        if(findRunningProcesses() == 0 && waitngQueue.size == 0 && findTotalInProcess == 0)
        {
            break; //breaks out of while loop
        }

        clock++;

    }

    int j;
    int endPid;
    for (j = 0; j < numberOfProcesses; j++)
    {
        totalTurnaround += processes[j].endTime - processes[j].arrivalTime;
        waitingTime += processes[j].waitingTime;


    }
    
/*
    printf("First Come First Serve Scheduling\n");
    printf("Avergae Waiting time is:\t\t %.3f \n", avgWaitTime(waitingTime));
    printf("Avergae Turnaround Time is:\t\t %.3f \n",avgTurnaroundTime(totalTurnaround));
    printf("CPU Completion time:\t\t %d \n", clock);
    printf("Average CPU Utilization Time:\t\t %.3f\n", avgUtilization(cpuUtilizedTime));
    printf("Total Amount of Context Switches:\t\t %d\n",contextSwitch);
*/

    printf("First Come First Serve Scheduling\n");
    printf("Avergae Waiting time is:\t\t %.3f \n", waitingTime / (double) numberOfProcesses);
    printf("Avergae Turnaround Time is:\t\t %.3f \n", totalTurnaround / (double) numberOfProcesses);
    printf("CPU Completion time:\t\t %d \n", clock);
    printf("Average CPU Utilization Time:\t\t %.3f\n", 100.0 * cpuUtilizedTime / clock);
    printf("Total Amount of Context Switches:\t\t %d\n",contextSwitch);
    printf("The Last process to finish is:");
    int i;
    for (i=0; i<numberOfProcesses;i++)
    {

        if(processes[i].endTime == clock)
        {
            endPid = processes[i].pid;
            printf("%d", endPid);
        }
    }
     printf("\n");
    return 0;
}