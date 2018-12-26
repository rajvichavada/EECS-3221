/*
#Chavada
#Rajvi
#214239800
#cse: rajvic
*/

#include<stdio.h>
#include <string.h>

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




int main (int argc, char *argv[])
{
	//initialize variables and Open file for reading
	
	int i;

for (i = 1; i <= argc-1; i++)
{
	FILE *fp;
	fp = fopen(argv[i], "r");

	//Error in opening file
	if(fp == NULL)
	{
		printf("ERROR in opening the file\n");
		fclose(fp);
	}


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
	qsort(wordList, numOfWords, sizeof(WordArray), cmp);
	printf("%s %d %s \n", argv[i], numOfWords, wordList[median].word);
	fclose(fp);


}

}
