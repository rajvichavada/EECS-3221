/*
#Chavada

#Rajvi

#214239800

#cse: rajvic

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "sch-helpers.h" 



process processes[MAX_PROCESSES+1];   // a large structure array to hold all processes read from data file 
int numberOfProcesses;              // total number of processes 
int nextProcess;
int clock;
int waitingTime;
int contextSwitch;
int cpuUtilizedTime;
//int completionTime; 


process_queue readyQueue;			//Create queue for all processes that are read
process_queue waitngQueue;			//Create queue for all processes that are waiting for I/O

process *CPUS[NUMBER_OF_PROCESSORS];	//CPUs
process *tempPreReady[MAX_PROCESSES];		//Creates a temprary queue that is used before the ready state
int tempQSize;


void resetVar(void)					// Will reset all the variables
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

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
 QUESTION 1: Method to calculate the average waiting for (FCFS Scheduling)

 */
double avgWaitTime(int waitingT)
{
	double avgTime;
	avgTime = waitingT / (double) numberOfProcesses;
	return avgTime;

}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
 QUESTION 2: Method to determine the turnaround time (time it takes for process to complete)

 */
double avgTurnaroundTime (int turnarround)
{
	double avgTurnaround;
	return avgTurnaround = turnarround / (double) numberOfProcesses;

}


/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
Question 3: Method to detrmine the time it takes for CPU to finish processes
*/
double avgUtilization (int cpuTime)
{
	double avgU;
	return avgU = 100.0 * cpuUtilizedTime / clock;
	
}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
Method to compare the PID to see which one has the lowest PID to be given priority
*/
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


/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
Method determines the total amount of CPUs that are running processes
CHECKED
*/
int findRunningProcesses(void)
{
	int running = 0;
	int i;
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++) 	//Iterate through all the cpus to find which ones have processes
	{
		if(CPUS[i] != NULL)		// if the CPU is not at a null state it has a processs
		{
			running++;		//hence increment the # of total Processes running
		}
	}
	return running;
}


/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
Method that returns the total amount of incoming processes. There processes have not been admitted into the scheudler/system yet.
CHECKED
*/
int findTotalInProcess(void)
{
	int result;
	result = numberOfProcesses - nextProcess;
	return result;
}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
STATE 1: New Process
Method takes any new incoming processes and adds them into a temporary queue so they can later be added into the ready queue.
CHECKED


*/
void addNewProcess(void)
{
	while (nextProcess < numberOfProcesses && processes[nextProcess].arrivalTime <= clock)
	{
		tempPreReady[tempQSize++] = &processes[nextProcess++];	//Incoming process placed in tempPreReady queue to be later used by ready queue
	}

}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
STATE 2: Ready Process. 
Returns Null if the ready queue is empty.
Gets the next process in the ready queue (from the front).
Removes the process from the ready queue and returns the next process.
CHECKED

*/
process *getNextProcess (void)
{
	if (readyQueue.size == 0)
	{
		return NULL; 	//Because there is nothing in the ready queue at the moment 
	}
	process *getNext = readyQueue.front->data; 	//get the element in the front of the ready queue (FIFO)
	dequeueProcess(&readyQueue);		//using library function to remove process from the front of the list 
	return getNext;

}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
TRANSITION: READY TO RUNNING 
Sorts the elements in the ready queue by their PIDs to avoid implications.
Enqueues all the process we have placed in the tempPreReady queue to the original ready queue.
Next finds a CPU whose value is not Null (meaning it is not running a process) and selects and schedules the 
next process for the CPU
CHECKED
*/
void readyToRunningProcess(void)
{
	qsort(tempPreReady, tempQSize, sizeof(process*), comparePIDs);		//sorts the temporary queue of processes based on their PID
	int j;
	for(j = 0; j < tempQSize; j++)
	{
		enqueueProcess(&readyQueue, tempPreReady[j]);	//add the sorted array into the ready queue using helper class function
	}
	tempQSize = 0;	//Reset the tempPreReady size to zero

	int k;
	for(k = 0; k < NUMBER_OF_PROCESSORS; k++)
	{
		if(CPUS[k] == NULL)		//if any of the CPUS are available then it will be assigned a process
		{
			CPUS[k] = getNextProcess(); //Assign the next process for the CPU
		}
	}
}

/*
 -----------------------------------------------------------------------------------------------------------------------------------------------
TRANSITION:  Waiting To Ready.
Iterates through the waiting queue to get the next process (FIFO - takes from front).
Dequeues this process from the waiting queue.
Checks if the process has completed its I/O burst.
If the I/O burst is completed, move the process to the tempPreReady queue and;
shift to the next I/O burst
CHECKED

*/
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
			getNext->currentBurst++;	//Move to the next I/O burst by updating the waiting state.
			tempPreReady[tempQSize++] = getNext; 	//Increase temp Q size by one and add the process that finished waiting
		}
		else {
            enqueueProcess(&waitngQueue, getNext);
        }
	}
}

//CHEcKED
void runningToWaitingProcess(void)
{
	int i;
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
		if(CPUS[i] != NULL) 	//CPU is running a process
		{
			

			//To check if the CPU burst has completed
			if(CPUS[i]->bursts[CPUS[i]->currentBurst].step ==
				CPUS[i]->bursts[CPUS[i]->currentBurst].length)
			{
				CPUS[i]->currentBurst++; //The processes next I/O burst
				
			

			if (CPUS[i]->currentBurst < CPUS[i]->numberOfBursts) {
				enqueueProcess(&waitngQueue, CPUS[i]); 	//If the running process has finished their CPU burst, move them back to 
														//the waiting queue for I/O bursts. 
			}

			else
			{
				CPUS[i]->endTime = clock; 		//Terminates current process by setting its time to the clock time
			}

				CPUS[i] = NULL;
			}	
		}
	}
}

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

void updateReady(void)
{
	int i;
		//to update the ready process queue
		for(i = 0; i < readyQueue.size; i++)
		{
			process *getNext = readyQueue.front->data;
			dequeueProcess(&readyQueue);
			getNext->waitingTime++;
			enqueueProcess(&readyQueue, getNext);
		}

}

void updateRunning(void)
{
	int i;
	//to update the cpu
		for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
		{
			if(CPUS[i] != NULL)
			{
				CPUS[i]->bursts[CPUS[i]->currentBurst].step++;
			}
		}
}


int main(void)
{
	int status = 0;
	int totalTurnaround=0;


	resetVar(); 	//Resets all global variables 



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
		updateRunning();
	

		

		cpuUtilizedTime += findRunningProcesses();

		//if there are no more processes to run
		if(findRunningProcesses() == 0 && waitngQueue.size == 0 && findTotalInProcess() == 0)
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
	printf("Avergae Waiting time is           :%.2f \n", avgWaitTime(waitingTime));
	printf("Avergae Turnaround Time is        :%.2f \n", avgTurnaroundTime(totalTurnaround));
	printf("CPU Completion time               :%d \n", clock);
	printf("Average CPU Utilization Time      :%.1f%%\n", avgUtilization(cpuUtilizedTime));
	printf("Total Amount of Context Switches  :%d\n",contextSwitch);
	printf("The Last process to finish is     :");

	for (j = 0; j < numberOfProcesses; j++)
	{
		if(processes[j].endTime == clock)
		{
			//endPid = processes[i].pid;
			printf("%d", processes[j].pid);
		}
		else
			printf("none");
	}
	 printf("\n");
	return 0;

}

/*
WORKS CITED:

https://www.thecrazyprogrammer.com/2014/11/c-cpp-program-for-first-come-first-served-fcfs.html
https://www.geeksforgeeks.org/program-fcfs-scheduling-set-1/
https://www.geeksforgeeks.org/program-fcfs-scheduling-set-2-processes-different-arrival-time/



*/