/*
 * Date: March 12, 2020
 * Author: David Richards
 * Class: CS4760
 * File: "master.c"
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sys/file.h>
#include <signal.h>
#include <semaphore.h>
#include <math.h>
#include <errno.h>

int shmid;
void sigErrors(int signum);
char resultFile[] = "adder_log";


/* Struct to hold integer shared memory arrays */
struct sharedMem
{
        int numbers[1024];
	int numbersLog[1024];
	int computationFlg;
};

struct sharedMem *intShared;

/* Set up shared memory */
void setUp()
{
        key_t mem_key = ftok("./master.c", 1);
        shmid = shmget(mem_key, sizeof(struct sharedMem), IPC_CREAT | 0666);

        if(shmid < 0)
        {
                printf("Error in shmget");
                exit(0);
        }

        intShared = (struct sharedMem*) shmat(shmid,NULL,0);
        if((intptr_t)intShared == -1)
        {
                printf("\n Error in shmat \n");
                exit(0);
        }
}

/* Detach shared memory segment */
void detach()
{
        shmdt((void*)intShared);
        shmctl(shmid, IPC_RMID, NULL);
}

/* Main program */
int main(int argc, char* argv[])
{
	/* Default random numbers to generate is 64  and alarm is 100 */
        int n = 64;
        int c;
	int timerAmount = 100;

	/* Using getopt to parse command line options for -h and -n */
        while((c=getopt(argc, argv, "n:t:h"))!= EOF)
        {
                switch(c)
                {
        case 'h':
                printf("\nInvocation: master [-h] [-n x -t x]\n");
                printf("-------------------------------------------------Program Options--------------------------------------------------\n");
                printf("       -h             Describe how the project should be run and then, terminate\n");
                printf("       -n x           Indicate amount of random numbers to generate (Default of 64)\n");
                printf("       -t x           Indicate alarm timer (Default of 100 seconds)(Do 200 seconds for 32 integers and 300 for 64)\n");
		return EXIT_SUCCESS;
        case 'n':
                n = atoi(optarg);
                break;
	case 't':
		timerAmount = atoi(optarg);
		break;
        default:
                return -1;

        	}
	}

	/* Set up shared memory */
        setUp();

	/* Open the inFile to write to it */
        FILE *file = fopen("intFile", "w");
        if(!file)
        {
                perror("Error could not open file\n");
                return EXIT_FAILURE;
        }

        srand(time(0));
        int i;
	
	/* Generate n random nmbers [0, 256) and print them to intFile */
        for(i = 0; i < n; i++)
        {
               int num = (rand() % (256 - 0 + 1)) + 0;
               fprintf(file, "%d\n", num);
        }

        fclose(file);


	/* Open intFile to read integers */
        fopen("intFile", "r");
        if(!file)
        {
                perror("Error opening file in master.c\n");
                return EXIT_FAILURE;
        }

	/* Read n numbers from intFile into the shared memory array */
        for(i = 0; i < n; i++)
        {
                fscanf(file, "%d", &intShared->numbers[i]);
        }

        fclose(file);

	/* Copy shared memory array into shared memory array for log computation*/
	for(i=0; i<n; i++)
    	{
        	intShared->numbersLog[i] = intShared->numbers[i];
    	}

	/* Signal handler for Cntrl-c and alarm(100) */
        if (signal(SIGINT, sigErrors) == SIG_ERR) //sigerror on cntrl-c
        {
                exit(0);
        }

        if (signal(SIGALRM, sigErrors) == SIG_ERR) //sigerror on program exceeding 2 second alarm
        {
                exit(0);
        }

	/* Create the semaphore used to protect the critical section  of n/2 */
        sem_t* sem;
        sem = sem_open("p3sem", O_CREAT, 0644, 1);
        if(sem == SEM_FAILED)
        {
                perror("Error in sem_open");
                exit(0);
        }
	
	/* Create the semaphore used to protect the critical section  of n/log(n) */
	sem_t* sem2;
	sem2 = sem_open("p3sem2", O_CREAT, 0645, 1);
	if(sem2 == SEM_FAILED)
	{
		perror("Error in sem_open for sem 2");
		exit(0);
	}
	
	/* Set the computation flag to 1 and start 100 second alarm */
	intShared->computationFlg = 1;
        alarm(timerAmount);

        int status;
        int numbers = n;
        int active = 1;
        int k;
        int count = n;
        pid_t pids[n],wpid;
	double launchChild = 0;
	int numbersTwo = n;
	launchChild = numbersTwo / 2;
	k = 0;
	int index1 = 0;
	double logNum;
    	logNum = 2;
    	int addNums = (int) logNum;
	int counter = 0;

	/* Get the current time for printing later */
        struct timeval time1, time2, time3, time4;
        gettimeofday(&time1, NULL);

	/* Write the header to adder_log for n/2 */
	FILE* logFile = fopen(resultFile, "a");
	fprintf(logFile, "n/2 computation\n                         PID\t\tIndex\t\tSize\t\tResult\t\tValues\n\n");
	
	fclose(logFile);
	//gettimeofday(&time3, NULL);
	
	printf("\nStarting n/2 computation\n");
	
	for (i=0; i < numbers / 2; i++)
	{
		while (index1 < numbersTwo - 1) 
		{
		k = 0;		
	  	pids[k] = fork(); //start forking processes
                                
				if(pids[k] == 0)
                                {
                                        char index[20]; //store index of numbers
                                        char yy[20]; //store count of numbers
                                        sprintf(index, "%d", index1);
                                        sprintf(yy, "%d", addNums);
                                        execl("./bin_adder",index,yy,NULL); //exec to bin_adder.c
                                        exit(0);
                                }

				index1 += (int) logNum;
				wpid = wait(&status);
				
		}
				intShared->numbers[1] = intShared->numbers[(int)logNum];
				
				int z;
        			for (z = 2; z < launchChild; z++) //shift array elements
				{
            				intShared->numbers[z] = intShared->numbers[z * (int)logNum];
        			}
			index1 = 0;
        		launchChild = launchChild / 2;
			
			if(launchChild == 1) //increment counter
			{
            			if(counter == 1)
				{
                			launchChild = 0;
           			}

            			counter +=1;
			}

			numbersTwo /= addNums;
        		logNum = 2;
        		addNums = 2;
	}
	
	/* Print total time taken for n/2 and the final result to the screen */
	gettimeofday(&time2, NULL);
	printf("\nTotal time taken for n / 2 processes: %f seconds\n", (double) (time2.tv_usec - time1.tv_usec) / 10000000 + (double) (time2.tv_sec - time1.tv_sec));
	printf("Final Result = %d\n ", intShared->numbers[0]);
	logFile = fopen(resultFile, "a");
	
	/* Print final result and header for n/log(n) to adder_log */
	fprintf(logFile, "Final Result = %d\n ", intShared->numbers[0]);	
	fprintf(logFile, "\n-----------------------------------------------------------------------------------------------------------------\n");
	fprintf(logFile, "n/log(n) computation:\n                         PID\t\tIndex\t\tSize\t\tResult\t\tValues\n\n");
	
	fclose(logFile);
	
	/* Set computation flag to 0 for second computation */
	intShared->computationFlg = 0;

	/* Reset variables for second computation */
	launchChild = 0;
	launchChild = (double) numbers / ceil(log2((double) numbers));
	k = 0;
	index1 = 0;
	logNum;
    	logNum = log2(numbers);
    	addNums = (int) logNum;
	counter = 0;
	
	printf("\n\nStarting n/log(n) computation\n");

	gettimeofday(&time3, NULL);

	while (launchChild > 0)
	{

		while (index1 < numbers) 
		{
			k = 0;		
	  		pids[k] = fork(); //start forking processes
                                
				if(pids[k] == 0)
                                {
                                        char xxLog[20]; //store index of numbers
                                        char yyLog[20]; //store count of numbers
                                        sprintf(xxLog, "%d", index1);
                                        sprintf(yyLog, "%d", addNums);
                                        execl("./bin_adder",xxLog,yyLog,NULL); //exec to bin_adder.c
                                        exit(0);
                                }

				index1 += (int) logNum;
				wpid = wait(&status);

		}
				intShared->numbersLog[1] = intShared->numbersLog[(int)logNum];
				
				int z;
        			for (z = 2; z < launchChild; z++) //shift array elements 
				{
            				intShared->numbersLog[z] = intShared->numbersLog[z * (int)logNum];
        			}
			index1 = 0;
        		launchChild = ceil(launchChild / 2);
			
			if(launchChild == 1) //increment counter
			{
            			if(counter == 1)
				{
                			launchChild = 0;
           			}

            			counter +=1;
			}
			
			numbers /= addNums;
        		logNum = 2;
        		addNums = 2; //always adding 2 numbers
	}
	
	gettimeofday(&time4, NULL);

	/* Print total time and final result for n/log(n) to screen */
        printf("\nTotal time taken for n / log(n) processes: %f seconds\n", (double) (time4.tv_usec - time3.tv_usec) / 10000000 + (double) (time4.tv_sec - time3.tv_sec));
	printf("Final Result = %d\n ", intShared->numbersLog[0]);	

	/* Print final result for n/log(n) to adder_log */
	logFile = fopen(resultFile, "a");
	fprintf(logFile, "Final Result = %d\n ", intShared->numbersLog[0]);
	fclose(logFile);
	
	/* Detach shared memory and unlink both semaphores */
	detach(); 
        sem_unlink("p3sem"); 
	sem_unlink("p3sem2");
        return 0;
}

/* Function for signal handling, either 100 second alarm or Ctrl-c */
void sigErrors(int signum)
{
        if (signum == SIGINT)
        {
                printf("\nSIGINT\n");
        }
        else
        {
                printf("\nSIGALRM\n");
        }
        shmctl(shmid, IPC_RMID, NULL);
        sem_unlink("p3sem");
	sem_unlink("p3sem2");
        kill(0, SIGTERM);
        exit(EXIT_SUCCESS);
}
