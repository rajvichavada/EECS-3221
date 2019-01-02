/*
#Chavada
#Rajvi
#214239800
#cse: rajvic
*/

#include<stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


typedef  struct {

     char  word[101] ;

     int   freq ;

} WordArray;


WordArray wordList [10000];
int numOfWords = 0;


// methode compare for the qsort function
int cmp (WordArray *w1, WordArray *w2)
{
	if ((*w1).freq < (*w2).freq)
	{
		return -1;
	}
	else if ((*w1).freq > (*w2).freq)
	{
		return 1;
	}
	else if ((*w1).freq == (*w2).freq)
	{
        return *(char *)w1 - *(char *)w2;
	}
	else
		return 0;
}

char *findMedian(char *fileName)
{
	FILE *fp;
	fp = fopen(fileName, "r");

	if (fp == NULL){
		printf("Couldn't open file: ");
	}

	else{

	//Make the 1st array
	char string[101];

	//Scan through each word and copy it in array
	while((fscanf(fp, "%s", string)) != EOF)
	{
		int i;
		int new = 1; 	//variable to indicate if a new word is encountered

		for(i = 0; i <= numOfWords; i++)
		{
			//compare the word with the ith word in the wordList dataset
			if (strcmp(wordList[i].word, string) == 0)
			{
				new = 0; 				//the word is in the set
				wordList[i].freq++; 	//Increase its freq
				break; 
			}
		}

		if (new == 1)
		{
			//If not in the set, add it in and set freq to one and increase num of words
			strcpy(wordList[numOfWords].word, string);
			wordList[numOfWords].freq = 1;
			numOfWords++;
		}
	}

	int median;

		if((numOfWords % 2) == 0)
		{
			median = numOfWords / 2;
		}
		else
		{
			median = ((numOfWords + 1) / 2);
		}


	//sort the words to find the most freq words using qsort amd cmp
	char *result = (char*)malloc(sizeof(result)*100);
	qsort(wordList, numOfWords, sizeof(WordArray), cmp);
	snprintf(result, 10000, "%s %d %s \n", fileName, numOfWords, wordList[median].word);
	fclose(fp);
	return result;
}
}

int main (int argc, char *argv[])
{
	int fd[argc-1][2];
	 pid_t  pids;
	 char *result = (char*)malloc(sizeof(char));
	 char *buff = (char*)malloc(sizeof(char));

	 //first check id user entered correct number of arguments
	 if(argc < 2)
	 {
	 	printf("Incorrect/Invalid Arguments");
	 	exit(0);
	 }

else
{
	int i;
	for (i = 0; i < argc; i++) 
	{
		//Create a new pipe
		pipe(fd[i]);
	
		if((pids = fork()) < 0) //Create a fork and comapre
		{
			perror("Fork");
			exit(0);
		}
		//Child process will send the result infor to parent process
		else if(pids == 0)
		{
			result = findMedian(argv[i]);
			//Read = 0; Write = 1
			close(fd[i][0]);
			write(fd[i][1], result, 1000);
			close(fd[i][1]);
			exit(0);

		}
	}
	//wait for child to go back to parent
	int j;
	for(j = 1; j <=argc-1; j++)
	{
		wait(NULL);
	}

	int k; 
	for (k = 1; k <= argc-1; k++)
	{
		//Reads from child and prints ou result 
		close(fd[k][1]);
		read(fd[k][0], buff, 1000);
		close(fd[k][0]);
		printf("%s\n", buff);
	}
}


}
