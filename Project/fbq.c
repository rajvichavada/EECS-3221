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
int timeSlice1;
int timeSlice2;


process_queue readyQueue1;
process_queue readyQueue2;
process_queue readyQueue3;			//Create queue for all processes that are read
process_queue waitngQueue;			//Create queue for all processes that are waiting for I/O

process *CPUS[NUMBER_OF_PROCESSORS];	//CPUs
process *tempPreReady[MAX_PROCESSES];		//Creates a temprary queue that is used before the ready state
int tempQSize;

void resetVar(void)					// Will reset all the variables
{
	int i;
    for (i=0;i<NUMBER_OF_PROCESSORS;i++) 
    {
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

	initializeProcessQueue(&readyQueue1);	//initialize the queues
	initializeProcessQueue(&readyQueue2);
	initializeProcessQueue(&readyQueue3);
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
Method to add to the front of the queue
*/
//To add the process to the front of the queue - method extracted from previous sch-helpers.c library found in course archives. 
void enqueueProcessHead(process_queue *q, process *p) {
  process_node *node = createProcessNode(p);
  if (q->front == NULL) {
      assert(q->back == NULL);
      q->front = q->back = node;
  }
  else {
      assert(q->back != NULL);
        node->next = q->front;
        q->front = node;
  }
  q->size++;

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
		tempPreReady[tempQSize] = &processes[nextProcess];	//Incoming process placed in tempPreReady queue to be later used by ready queue
		//tempPreReady[tempQSize]->quantumRemaining = timeQ;
		tempQSize++;
		nextProcess++;
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
	process *getNext;

	if (readyQueue1.size == 0 || readyQueue2.size == 0 || readyQueue3.size ==0)
	{
		return NULL; 	//Because there is nothing in the ready queue at the moment 
	}
	

	

	if (readyQueue1.size != 0)
	{
		getNext = (*readyQueue1.front).data; 	//get the element in the front of the ready queue (FIFO)
		dequeueProcess(&readyQueue1);		//using library function to remove process from the front of the list 
		//return getNext;
	}
	else if (readyQueue2.size != 0)
	{
		getNext = (*readyQueue2.front).data; 	//get the element in the front of the ready queue (FIFO)
		dequeueProcess(&readyQueue2);	
	}
	else if (readyQueue3.size != 0)
	{
		getNext = (*readyQueue3.front).data; 	//get the element in the front of the ready queue (FIFO)
		dequeueProcess(&readyQueue3);	
	}

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
		tempPreReady[j]->currentQueue = 0;
		enqueueProcess(&readyQueue1, tempPreReady[j]);	//add the sorted array into the ready queue using helper class function
	}
	tempQSize = 0;	//Reset the tempPreReady size to zero

	int k;
	for(k = 0; k < NUMBER_OF_PROCESSORS; k++)
	{
		if(CPUS[k] == NULL)		//if any of the CPUS are available then it will be assigned a process
		{
			if(readyQueue1.size != 0)
			{
				CPUS[k] = (*readyQueue1.front).data; 
				dequeueProcess(&readyQueue1);
			}
			else if(readyQueue2.size != 0)
			{
				CPUS[k] = (*readyQueue2.front).data; 
				dequeueProcess(&readyQueue2);
			}
			else if (readyQueue3.size != 0)
			{
				CPUS[k] = (*readyQueue3.front).data; 
				dequeueProcess(&readyQueue2);
			}

			//CPUS[k] = getNextProcess(); //Assign the next process for the CPU
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

HERE FOR TIME QUANTUM REMAINING 

*/
void waitingToReadyProcess(void)
{
	int i;
	int s = waitngQueue.size;
	for(i = 0; i < s; i++)
	{
		process *getNext = (*waitngQueue.front).data;
		getNext->priority = 0;
		dequeueProcess(&waitngQueue);

		// assert(getNext->bursts[getNext->currentBurst].step <= getNext->bursts[getNext->currentBurst].length);

		//Check if no. of steps completed is equal to time steps required for the burst
		if(getNext->bursts[getNext->currentBurst].step == 
			getNext->bursts[getNext->currentBurst].length)
		{
			(*getNext).currentBurst++;	//Move to the next I/O burst by updating the waiting state.
			(*getNext).quantumRemaining = timeSlice1; //reset the first time quantum for the next cpu burst that occurs in the schedule		//getNext->endTime = clock;
			tempPreReady[tempQSize] = getNext; 	//Increase temp Q size by one and add the process that finished waiting
			tempQSize++;
		}
		else {
            enqueueProcess(&waitngQueue, getNext); //put back into waiting queue
        }
	}
}

//CHEcKED
void runningToWaitingProcess(void)
{
	int i;
	int index = 0;
	process *ps[NUMBER_OF_PROCESSORS];

	
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
	{
		if(CPUS[i] != NULL) 	//CPU is running a process
		{
			//check is the current CPUS burst has finished
			if(CPUS[i]->bursts[CPUS[i]->currentBurst].step != CPUS[i]->bursts[CPUS[i]->currentBurst].length)
			{
				CPUS[i]->currentBurst++;

				//secondary check to see if we are not exceding no. of burst/clock
				if (CPUS[i]->currentBurst < CPUS[i]->numberOfBursts)
				{
					enqueueProcess(&waitngQueue, CPUS[i]);
				}
				else
				{
					CPUS[i]->endTime = clock;
				}
				CPUS[i] = NULL; //reset cpus
			}
		

		//If the current cpu burst is not finised and the time quantum is finished
		else if (CPUS[i]->quantumRemaining == 0)
		{
			ps[index] = CPUS[i];
			index++;
			contextSwitch++;
			CPUS[i] = NULL;
		}

		//quantime time is not yet done
		else if (CPUS[i]->quantumRemaining != 0)
		{
			if(readyQueue1.size != 0)
			{
				if(CPUS[i]->currentQueue == 1)
				{
					enqueueProcessHead(&readyQueue1, CPUS[i]); //add to front of ready queue
					CPUS[i] = (*readyQueue1.front).data;
					dequeueProcess(&readyQueue1);
					contextSwitch++; 	//increment context switch 
				}
				else if(CPUS[i]->currentQueue == 2)
				{
					enqueueProcessHead(&readyQueue2, CPUS[i]);
					CPUS[i] = (*readyQueue1.front).data;
					dequeueProcess(&readyQueue1);
					contextSwitch++;
				}
			}
			else if (readyQueue2.size != 0 && CPUS[i]->currentQueue == 2)
			{
				enqueueProcessHead(&readyQueue3, CPUS[i]);
				CPUS[i] = (*readyQueue2.front).data;
				dequeueProcess(&readyQueue2);
				contextSwitch++;
			}
		}
		}

		//The cpu is not running a process and needs to be assigned a process from one of the 3 levels
		else if (CPUS[i] == NULL)
		{
			if (readyQueue1.size != 0)
			{
				CPUS[i] = (*readyQueue1.front).data;
				dequeueProcess(&readyQueue1);
			}
			else if(readyQueue2.size != 0)
			{
				CPUS[i] = (*readyQueue2.front).data;
				dequeueProcess(&readyQueue2);
			}
			else if (readyQueue3.size != 0)
			{
				CPUS[i] = (*readyQueue3.front).data;
				dequeueProcess(&readyQueue3);
			}
		}
	}

qsort(ps, index, sizeof(process*), comparePIDs); 	//sort processes by pid

for (i = 0; i < index; i++)
{
	if(ps[i]->bursts[ps[i]->currentBurst].step == timeSlice1)
	{
		ps[i]->quantumRemaining = timeSlice2; //assign to second time quantum
		ps[i]->currentQueue = 1;
		enqueueProcess(&readyQueue2, ps[i]);
	}
	//to avoid starvation
	else if(ps[i]->bursts[ps[i]->currentBurst].step == (timeSlice1+timeSlice2))
	{
		ps[i]->currentQueue = 2;
		enqueueProcess(&readyQueue3, ps[i]); 	//add to the 3rd ready Q
	}
}
			
	/*qsort(pS, value, sizeof(process*), comparePIDs);
	for(i = 0; i < value; i++)
	{
		enqueueProcess(&readyQueue, pS[i]);
	}
	*/

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
		for(i = 0; i < readyQueue1.size; i++)
		{
			process *getNext = (*readyQueue1.front).data;
			dequeueProcess(&readyQueue1);
			getNext->waitingTime++;
			enqueueProcess(&readyQueue1, getNext);
		}
		//to update the ready process queue
		for(i = 0; i < readyQueue2.size; i++)
		{
			process *getNext = (*readyQueue2.front).data;
			dequeueProcess(&readyQueue2);
			(*getNext).waitingTime++;
			enqueueProcess(&readyQueue2, getNext);
		}
		//to update the ready process queue
		for(i = 0; i < readyQueue3.size; i++)
		{
			process *getNext = (*readyQueue3.front).data;
			dequeueProcess(&readyQueue3);
			(*getNext).waitingTime++;
			enqueueProcess(&readyQueue3, getNext);
		}

}

void updateRunning(void)
{
	int i;
	//to update the cpu thats running
		for(i = 0; i < NUMBER_OF_PROCESSORS; i++)
		{
			if(CPUS[i] != NULL) 	//cpu has a process
			{
				CPUS[i]->bursts[CPUS[i]->currentBurst].step++;
				(*CPUS[i]).quantumRemaining--; //decrease the quantum
			}
		}
}


int main(int argc, char *argv[])
{	int i=0;
	int status = 0;
	int totalTurnaround=0;
	int totalWait=0;
	timeSlice1 = atoi(argv[1]); //1st time slice
	timeSlice2 = atoi(argv[2]);	//2nd time quantum

	//Throwing statements to catch input errors from the user
	if(argc > 3)
	{
		printf("Incorrect number of time slices\n");
		exit(0);
		
	}
	else if(argc < 3)
	{
		printf("Missing timeslice or loading data\n");
		exit(0);
		
	}


	resetVar(); 	//Resets/initializes all global variables 


	//read from the file
	while ((status=(readProcess(&processes[numberOfProcesses]))))
	{
         if(status==1)
         {
         	numberOfProcesses ++;
         }

         //if proccesses exceed the set max then break out of loop
         if(numberOfProcesses > MAX_PROCESSES)
         {
         	break;
         }

	}   

	//if there are no processes print error msg
	 if (numberOfProcesses == 0) 
	 {
        fprintf(stderr, "Error: no processes.\n");
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

		//update the states
		updateWaiting();
		updateReady();
		updateRunning();
	

		

		cpuUtilizedTime += findRunningProcesses();

		//if there are no more processes to run
		if(findRunningProcesses() == 0 && waitngQueue.size == 0 && findTotalInProcess() == 0)
		{
			break; //breaks out of while loop
		}

		//increasae clock when moving to next step
		clock++;

	}

	int j;
	int endPid;
	//calculations for waiting time and total turnaround values
	for (j = 0; j < numberOfProcesses; j++)
	{
		totalTurnaround += processes[j].endTime - processes[j].arrivalTime;
		totalWait += processes[j].waitingTime;
		

	}
	
	for (j = 0; j < numberOfProcesses; j++)
	{
		if (processes[j].endTime == clock)	//to find the last proess
		{
			endPid = processes[i].pid;
		}
		
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
	printf("Avergae Waiting time is           :%.2f \n", avgWaitTime(totalWait));
	printf("Avergae Turnaround Time is        :%.2f \n", avgTurnaroundTime(totalTurnaround));
	printf("CPU Completion time               :%d \n", clock);
	printf("Average CPU Utilization Time      :%.1f%%\n", avgUtilization(cpuUtilizedTime));
	printf("Total Amount of Context Switches  :%d\n",contextSwitch);
	printf("The Last process to finish is     :%.d\n", endPid);

	
	return 0;

}


/*
WORKS CITED:

https://www.thecrazyprogrammer.com/2014/11/c-cpp-program-for-first-come-first-served-fcfs.html
https://www.geeksforgeeks.org/program-fcfs-scheduling-set-1/
https://www.geeksforgeeks.org/program-fcfs-scheduling-set-2-processes-different-arrival-time/
https://www.geeksforgeeks.org/program-round-robin-scheduling-set-1/
https://www.thecrazyprogrammer.com/2015/09/round-robin-scheduling-program-in-c.html
https://www.quora.com/How-do-you-implement-a-C-program-for-round-robin-scheduling-with-arrival-time
https://interviewpreparation4u.com/round-robin-scheduling-algorithm-c-and-c-program/

http://www.codingalpha.com/multi-level-feedback-queue-scheduling-c-program/
https://www.geeksforgeeks.org/multilevel-feedback-queue-scheduling/
https://www.quora.com/How-can-I-implement-a-Multi-level-feedback-queue-CPU-scheduler
*/
