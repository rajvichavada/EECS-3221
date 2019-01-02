void cpuOut(void) {
	int i;
  int tempIndex = 0;
  process *temp[NUMBER_OF_PROCESSORS];
	for(i = 0; i < NUMBER_OF_PROCESSORS; i++) {
		// Note that initiallly there is no cpu running on any processes.
		if(cpus[i] != NULL) {
			/* check if current burst is finished */
			if(isBurstFinished(cpus[i])) {
				(cpus[i]->currentBurst)++;
				/* do another check for endtime or just a enqueue step */
				if((cpus[i])->currentBurst < (cpus[i])->numberOfBursts) {
					enqueueProcess(&device_queue, cpus[i]);
				}
				else {
					cpus[i]->endTime = clockTime;
				}
				/* free current working cpu */
				cpus[i] = NULL;
      }
      // If the current cpu burst is not finised but the time quantum is running out.

			else if(cpus[i]->quantumRemaining == 0) {
				temp[tempIndex] = cpus[i];
				tempIndex++;
				totalCW++;
				cpus[i] = NULL;
			}
			else if(cpus[i]->quantumRemaining != 0) {
					if(ready_queue[0].size != 0) {
						if(cpus[i]->currentQueue == 1 || cpus[i]->currentQueue == 2) {
							enqueueProcessHead(&ready_queue[cpus[i]->currentQueue], cpus[i]);
							cpus[i] = ready_queue[0].front->data;
							dequeueProcess(&ready_queue[0]);
							totalCW++;
						}
					}
					else if(ready_queue[1].size != 0 && cpus[i]->currentQueue == 2) {
						enqueueProcessHead(&ready_queue[2], cpus[i]);
						cpus[i] = ready_queue[1].front->data;
						dequeueProcess(&ready_queue[1]);
						totalCW++;
					}
			}
		}
		else if(cpus[i] == NULL) {
			if(ready_queue[0].size != 0) {
				cpus[i] = ready_queue[0].front->data;
				dequeueProcess(&ready_queue[0]);
			}
			else if(ready_queue[1].size != 0) {
				cpus[i] = ready_queue[1].front->data;
				dequeueProcess(&ready_queue[1]);
			}
			else if(ready_queue[2].size != 0) {
				cpus[i] = ready_queue[2].front->data;
				dequeueProcess(&ready_queue[2]);
			}
		}

	}
	/* like tempArray in i/o to ready_queue function call, there might be several processes that need
	*  to be added to the tail of ready_queue, so we need to sort them by the process id.
	*/
	qsort(temp, tempIndex, sizeof(process *), compareByPid);
	for(i = 0; i < tempIndex; i++) {
		if(temp[i]->bursts[temp[i]->currentBurst].step == timeQuantum1) {
			temp[i]->quantumRemaining = timeQuantum2;
			temp[i]->currentQueue = 1;
			enqueueProcess(&ready_queue[1], temp[i]);
		}
		else if(temp[i]->bursts[temp[i]->currentBurst].step == (timeQuantum1+timeQuantum2)) {
			temp[i]->currentQueue = 2;
			enqueueProcess(&ready_queue[2], temp[i]);
		}
	}

}