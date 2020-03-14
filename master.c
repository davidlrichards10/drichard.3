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
char fon[] = "adder_log";


/* Struct to hold integer shared memory array */
struct sharedMem
{
        int numbers[1024];
	int numbersLog[1024];
	int numbersSum[1024];
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
        int n = 64; //default random numbers to generate is 64
        int c;

	/* Using getopt to parse command line options for -h and -n */
        while((c=getopt(argc, argv, "n:h"))!= EOF)
        {
                switch(c)
                {
        case 'h':
                printf("\nInvocation: master [-h] [-n x]\n");
                printf("-------------------------------------------------Program Options--------------------------------------------------\n");
                printf("       -h             Describe how the project should be run and then, terminate\n");
                printf("       -n x           Indicate amount of random numbers to generate (Default of 64)\n");
                return EXIT_SUCCESS;
        case 'n':
                n = atoi(optarg);
                break;
        default:
                return -1;

        	}
	}


        setUp(); //setup shared memory

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

	/* Copy shared memory array into shared memory array for sum output*/
        for(i=0; i<n; i++)
        {
                intShared->numbersSum[i] = intShared->numbers[i];
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
	
	sem_t* sem2;
	sem2 = sem_open("p3sem2", O_CREAT, 0645, 1);
	if(sem2 == SEM_FAILED)
	{
		perror("Error in sem_open for sem 2");
		exit(0);
	}
		intShared->computationFlg = 1;

                //alarm(100); //set alarm to terminate after 100 seconds
                int status;
                int numbers = n;
                int active = 1;
                int k;
                int count = n;
                pid_t pids[n],wpid;

		printf("\nStarting n/2 computation\n");

		struct timeval tv1, tv2, tv3, tv4;
		gettimeofday(&tv1, NULL);

		/* Begin computation of n/2 */
                while(k < numbers - 1)
                {
                        k = 0;
                        if(active < 2)
                        {
                                pids[k] = fork(); //start forking processes
                                if(pids[k] == 0)
                                {
                                        char index[20]; //store index of numbers
                                        char yy[20]; //store count of numbers
                                        sprintf(index, "%d", k);
                                        sprintf(yy, "%d", count);
                                        execl("./bin_adder",index,yy,NULL); //exec to bin_adder.c
                                        exit(0);
                                }
                                active++;
                                k+=1;
                                n--;
                                if(active == 2){

                                        int m;
                                        for(m = 0; m < n; m++){
                                                pid_t tempId = waitpid(pids[m], &status, WNOHANG);
                                                if(tempId == 0){
                                                        waitpid(tempId, &status, 0);
                                                        active--;
                                                        break;
                                                }
                                        }
                                }

                                if(k > numbers - 1){
                                        while((wpid = wait(&status)) > 0);
                                        break;
                                }
                                if (k > numbers)
                                {
                                        break;
                                }
                        }
                        count = count / 2;
                        if (count == 1) //break when count is equal; to 1
                        {
                                break;
                        }
                }
	
	gettimeofday(&tv2, NULL);
	printf("\nTotal time taken for n / 2 processes: %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 10000000 + (double) (tv2.tv_sec - tv1.tv_sec));
	int sum = 0;
	int t;
	for(i=0; i<numbers; i++)
    	{
        	sum = sum + intShared->numbersSum[i];
    	}
	printf("Final Result = %d", sum);
	printf("\n\nStarting n/log(n) computation\n");
	FILE* logFile = fopen(fon, "a");
	fprintf(logFile, "\nFinal Result = %d", sum);
	fprintf(logFile, "\n-------------------------------------------------------------------------------------------------------\n");
	fclose(logFile);
	/* Reset components for second computation*/
	intShared->computationFlg = 0;

	double launchChild = 0;
	launchChild = ceil((double) numbers / log2((double) numbers));
	k = 0;
	int index1 = 0;
	double logNumberDistance;
    	logNumberDistance = log2(numbers);
    	int logNumbersToAdd = (int) logNumberDistance;
	int loopCounter = 0;

	gettimeofday(&tv3, NULL);
	while (launchChild > 0)
	{

		while (index1 < numbers) 
		{
		k = 0;		
	  	pids[k] = fork(); //start forking processes
                                
				if(pids[k] == 0)
                                {
                                        char index[20]; //store index of numbers
                                        char yy[20]; //store count of numbers
                                        sprintf(index, "%d", index1);
                                        sprintf(yy, "%d", logNumbersToAdd);
                                        execl("./bin_adder",index,yy,NULL); //exec to bin_adder.c
                                        exit(0);
                                }

				index1 += (int) logNumberDistance;
				wpid = wait(&status);

		}
				intShared->numbersLog[1] = intShared->numbersLog[(int)logNumberDistance];
				
				int z;
        			for (z = 2; z < launchChild; z++) 
				{
            				intShared->numbersLog[z] = intShared->numbersLog[z * (int)logNumberDistance];
        			}
			index1 = 0;
        		launchChild = ceil(launchChild / 2);
			
			if(launchChild == 1)
			{
            			if(loopCounter == 1)
				{
                			launchChild = 0;
           			}

            			loopCounter +=1;
			}
			numbers /= logNumbersToAdd;
        		logNumberDistance = 2;
        		logNumbersToAdd = 2;
	}
	
	gettimeofday(&tv4, NULL);

        printf("\nTotal time taken for n / log(n) processes: %f seconds\n", (double) (tv4.tv_usec - tv3.tv_usec) / 10000000 + (double) (tv4.tv_sec - tv3.tv_sec));
	
	logFile = fopen(fon, "a");
	
	printf("Final Result = %d\n", sum);
	fprintf(logFile, "\nFinal Result = %d", sum);
	fclose(logFile);
	
	detach(); //detach shared memory
        sem_unlink("p3sem"); //unlink semaphore
	sem_unlink("p3sem2");
        return 0;
}

void sigErrors(int signum) //function for signal handling, either 100 second alarm or Ctrl-c
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
        kill(0, SIGTERM);
        exit(EXIT_SUCCESS);
}
