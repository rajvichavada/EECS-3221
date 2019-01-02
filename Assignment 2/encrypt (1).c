/*

Family Name: Chavada

Given Name:	Rajvi

Section: Z

Student Number: 214239800

CS Login: rajvic

*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define TEN_MILLIS_IN_NANOS 10000000

typedef  struct {

     char  data ;

     off_t offset ;

     char state;   

} BufferItem;

struct timespec t;


BufferItem *bItem;
FILE *file_in;
FILE *file_out;
int bufferSize=0;

//Create our mutex locks for the threads to od mutual exclusion 
pthread_mutex_t mutexIN;
pthread_mutex_t mutexOUT;
pthread_mutex_t mutexWORK;

//To keep track of all the active threads
int activeIN;
int activeWORK;
int activeOUT;


/*

Method Description: Checks to see if the buffer is empty
Return type: True/False(0) based on if its empty or not
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

int emptyBuffer()
{
	int i = 0;
	for(i = 0; i < bufferSize; i++)
	{
		if (bItem[i].state == 'e')
		{
			return i;	//empty buffer exists
		}
		
			
	}
	return 0; 	//empty buffer does not exist

}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: Initializes the buffer to be empty 
Return type: No return
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

void initBuffer()
{
	int i;
	for (i = 0; i < bufferSize; i++)
	{
		bItem[i].state = 'e';
	}
}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: Checks to see if the buffer is empty
Return type: Buffer item to be worked on or 0 if there is no buffer with stae w
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

int workBuffer()
{
	int i;
	for (i = 0; i < bufferSize; i++)
	{
		if(bItem[i].state == 'w')
		{
			return i;	//returns the buffer with state w to do some work		}
		}
		
	}
	return 0;	//returns 0 if theres no working buffers

}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: To determine the encrypted item to be outputted to the file_out from buffer
Return type: Encrypted Buffer item to be sent to file_out or 0 if there is no buffer with stae o
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
int outBuffer()
{
	int i;
	for (i = 0; i < bufferSize; i++)
	{
		if(bItem[i].state == 'o')
		{
			return i; 	
		}
		
	}
	return 0;
}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: Puts the thread to sleep for some random time between 0 and 0.01 seconds
Return type: None.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/
void put_thread_to_sleep(void)
{
	//Extracted from A2 Hint file.
	int seed = 0;
	t.tv_sec = 0;
	t.tv_nsec = rand_r ((unsigned int*)&seed) % (TEN_MILLIS_IN_NANOS + 1);
	nanosleep(&t, NULL);
}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: 
Return type: None.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

void decreaseThreads(int active)
{
	pthread_mutex_lock(&mutexWORK);
	active = active - 1; //decrease the number of active threads before moving onto next one
	pthread_mutex_unlock(&mutexWORK);
}

void *thread_IN(void *param)
{
	int i;
	char c; 
	off_t offset;

	//Each IN thread goes to sleep (use nanosleep) for some random time between 0 and 0.01 seconds upon being created.
	put_thread_to_sleep();

	while(!feof(file_in))
	{	
		i = emptyBuffer(); //read from file
		pthread_mutex_lock(&mutexWORK); //the first critical section where we must add into he wprk lock
		

		while(i > 0)	//while empty buffers exist
		{
			//if the buffer is not empty (full) it will go to sleep
			if(emptyBuffer() ==  0)
			{
				put_thread_to_sleep();
			}

			//the second critical section where we will read from file and add to buffer
			pthread_mutex_lock(&mutexIN);
			offset = ftell(file_in);
			c = fgetc(file_in);			//get the character from the file
			pthread_mutex_unlock(&mutexIN);

			//need to check if the character was the end of file to break out of loop before we continue
			if (c == EOF)
			{
				break;
			}
			else
			{
				bItem[i].offset = offset;
				bItem[i].state = 'w'; 	//change buffer from empty to w once we retrieve character
				bItem[i].data = c; 
				i = emptyBuffer(); //gets the next empty item in the buffer until EOF is reached
			}
		}

		//Since there are no more empty buffers need to unlock
		pthread_mutex_unlock(&mutexWORK);

	}

	//Then, this IN threads goes to sleep (use nanosleep) for some random time between 0 and 0.01 seconds and then goes back to read the next byte of the file until the end of file.
	put_thread_to_sleep();
	decreaseThreads(activeIN);
	return NULL;
}




/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: this is the work buffer where we encrypt the character and write it back to the buffer
Return type: None.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

void *thread_WORK (void *param)
{

	int i;
	char data;
	int key = atoi(param);
	int activeInput;

	put_thread_to_sleep();	//put to sleep first like the questions stated

	while(i > 0 || activeInput > 0)
	{
		i = workBuffer();	//start of critical sections
		pthread_mutex_lock(&mutexWORK);

		if (i > 0)
		{
			data = bItem[i].data;	//store the read character into the buffer

			//if the buffer is not empty (full) it will go to sleep
			if(emptyBuffer() ==  0)
			{
				put_thread_to_sleep();
			}

			//need to check if the character was the end of file to break out of loop before we continue
			if (data == EOF)
			{
				break;
			}

			//if the key is positive we need to encrypt
			if (key > 0 && data > 31 && data < 127)
			{
				data = (((int)data-32)+2*95+key)%95+32;
			}
			//if key is negative we need to decrypt
			else if(key < 0 && data > 31 && data < 127)
			{
				data = (((int)data-32)+2*95-(key*-1))%95+32;
			}

			//start of critical section
			bItem[i].data = data;	//write encrypted data to tehe buffer
			bItem[i].state = 'o';	//change to o to show that its ready to be outputted 


		}

		activeInput = activeIN;
		pthread_mutex_unlock(&mutexWORK);
	}
	put_thread_to_sleep();

	decreaseThreads(activeWORK);
	return NULL;
}

/*
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Method Description: this is the out buffer where we send encrypted file to output
Return type: None.
-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
*/

void *thread_OUT(void *param)
{
	int i = 0;
	char c;
	off_t offset;
	int activeWorkTemp;

	put_thread_to_sleep();

	while(i > 0 || activeWorkTemp > 0)
	{
		//start of critical section to read char to be outputted
		i = outBuffer();
		pthread_mutex_lock(&mutexWORK);

		if( i > 0)
		{
			c = bItem[i].data;		//get the character that we want to output
			offset = bItem[i].offset;	//where we want the charcter to go

			//critical section to write to the output file
			pthread_mutex_lock(&mutexOUT);

			if(fseek(file_out, offset, SEEK_SET) == -1)
			{
				fprintf(stderr, "ERROR Usage: error setting output file" );
				exit(1);
			}

			if (fputc(c, file_out) == EOF)
			{
				fprintf(stderr, "ERROR Usage: error in writing to output file");
				exit(1);
			}

			pthread_mutex_unlock(&mutexOUT);

			//we emptied the buffer by writing the char to the output file
			//so we need to chnage the state to e for the next item
			bItem[i].data = '\0';
			bItem[i].state = 'e';
			bItem[i].offset = 0;

		}

		activeWorkTemp = activeWORK;
		pthread_mutex_unlock(&mutexWORK);
	}
	return NULL;
}



void initThreads()
{
	//initialize the buffers
	pthread_mutex_init(&mutexWORK, NULL);
	pthread_mutex_init(&mutexIN, NULL);
	pthread_mutex_init(&mutexOUT, NULL);
}

void destroyThreads()
{
	//destroys exisiting threads
	pthread_mutex_destroy(&mutexIN);
	pthread_mutex_destroy(&mutexOUT);
	pthread_mutex_destroy(&mutexWORK);

}



//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[])
{
	int key, nIN, nOUT, nWORK;
	int i;


	//need to read in the file arguments
	file_in = fopen(argv[5],"r");	//file to encrypt
	file_out = fopen(argv[6],"write");	//file to send the encryption to
	key = atoi(argv[1]);
	nIN = atoi(argv[2]);
	nWORK = atoi(argv[3]);
	nOUT = atoi(argv[4]);
	bufferSize = atoi(argv[7]);
	activeIN = nIN;
	activeWORK = nWORK;

	//First we need to check if there is a correct number of argume
	if(argc < 8)
	{
		printf("ERROR Usage: encrypt <KEY> <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize>");
		exit(1);
	}

	//Next we need to check if the key is in the range
	if (key < -127 || key > 127)
	{
		printf("ERROR Usage: Key is not in valud range -127 < key < 127");
		exit(1);
	}

	//Next we need to check if nIN, nWORK, nOUT are at least 1
	if (nIN < 1 || nWORK < 1 || nOUT < 1 || bufferSize < 1)
	{
		printf("ERROR Usage: <nIN> <nWORK> <nOUT> <file_in> <file_out> <bufSize> has to at least be of size 1");
		exit(1);
	}

	bItem = (BufferItem*) malloc(sizeof(BufferItem)* bufferSize); //need to allocate memory for respective buffer size
	initBuffer();


	pthread_t threadIN[nIN];
	pthread_t threadOUT[nOUT];
	pthread_t threadWORK[nWORK];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	initThreads();

	//Need to create specified number of threads
	for(i = 0; i < nIN; i++)
	{
		pthread_create(&threadIN[i], &attr, (void *) thread_IN, file_in);

	}
	for(i = 0; i < nWORK; i++)
	{
		pthread_create(&threadWORK[i], &attr,(void *) thread_WORK, argv[1]);
	}
	for(i = 0; i < nOUT; i++)
	{
		pthread_create(&threadOUT[i], &attr, (void *) thread_OUT, file_out);
	}


	//join the threads
	for(i = 0; i < nIN; i++)
	{
		pthread_join(threadIN[i], NULL);

	}
	for(i = 0; i < nWORK; i++)
	{
		pthread_join(threadWORK[i], NULL);
	}
	for(i = 0; i < nOUT; i++)
	{
		pthread_join(threadOUT[i], NULL);
	}

	//destroy created threads
	destroyThreads();

	// close all files and free buffer
	fclose(file_in);
	fclose(file_out);
	free(bItem);
	return 0;
}



//----------------------------------------------------------------------------------------------------------------------------------------
/*
	WORKS CITED:
	https://www.cs.cmu.edu/afs/cs/academic/class/15492-f07/www/pthreads.html
	https://www.thegeekstuff.com/2012/04/create-threads-in-linux/
	https://wiki.eecs.yorku.ca/course_archive/2018-19/F/3221/assignments
	https://www.geeksforgeeks.org/multithreading-c-2/
	https://users.cs.cf.ac.uk/Dave.Marshall/C/node32.html
	http://www.csc.villanova.edu/~mdamian/threads/posixthreads.htmlhttp://github.com




*/