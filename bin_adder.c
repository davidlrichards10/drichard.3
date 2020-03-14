/*
 * Date:   March 12, 2020
 * Author: David Richards
 * Class:  CS4760
 * File: "bin_adder.c"
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#include <math.h>

struct sharedMem
{
        int numbers[1024];
	int numbersLog[1024];
	int numbersSum[1024];
	int computationFlg;
};

struct sharedMem *intShared;

int main(int argc, char *argv[])
{

	/* Set up clock to determine timing of processes */
        struct timespec tyme;
        clock_gettime(CLOCK_MONOTONIC, & tyme);
        srand((unsigned)(tyme.tv_sec ^ tyme.tv_nsec ^ (tyme.tv_nsec >> 31)));
	
        sem_t* sem;
	sem_t* sem2;

	/* Open semaphore to protect critical section */
        sem = sem_open("p3sem",0);
        if(sem == SEM_FAILED)
        {
                perror("./bin_adder: Error in sem_open");
                exit(0);
        }
	
	sem2 = sem_open("p3sem2",0);
	if(sem2 == SEM_FAILED)
	{
		perror("./bin_adder: Error in sem_open sem2");
		exit(0);
	}

	/* Allocate shared memory */
 	key_t mem_key = ftok("./master.c", 1);
        int shmid2 = shmget(mem_key, sizeof(struct sharedMem), 0666);
        if(shmid2 == -1)
        {
                perror("Error in bin_adder shmget\n");
                exit(0);
        }

	/* Attach shared memory */
        intShared = shmat(shmid2,(void *)0,0);
        if((intptr_t) intShared == -1)
        {
                perror("Error in bin_adder shmat\n");
                exit(0);
	}

	/* index is exec argument 1 and count is exec argument 2 */
        int index = atoi(argv[0]);
        int count = atoi(argv[1]);

	/* Variables to update shared memory array */
        int i = 0;
        int beginNum;
        int computationNum;
	int beginNumLog;
	int computationNumLog;

	/* Open adder_log to update it as the program runs */
        FILE* file1;
        file1 = fopen("adder_log", "a");

	/* Set timer */
        const time_t tma = time(NULL);
        char * tme = ctime( & tma);
	
	/* Starting values of the variables to update the shared memory array */
        beginNum = index;
        computationNum = index + (count / 2);

	int counter = 0;	

	if (intShared->computationFlg == 1)
	{
	
	/* Loop for critical processes and display information regarding processes */
        for (i = 0; i < (count / 2); i++, beginNum++, computationNum++)
	{
		int result = intShared->numbers[beginNum] + intShared->numbers[computationNum];
                int num = (rand()%4); // store and create random number between 0 and 3 for sleep call
		
                sleep(num); // Sleep for a random amount of time bewteen 0 and 3 seconds
                fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem); //wait for semaphore
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
                wait(1); //wait 1 second before writing to the file
		fprintf(file1, "n / 2 computation:      PID: %d     Index: %d     Size: %d    Result: %d\n", getpid(), (index + i), count,result);
		wait(1); //wait one second before leaving the critical section
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem); //signal the semaphore
                intShared->numbers[beginNum] = result; //set result back
        }

	fclose(file1);
	}
	
	else
	{
		int numLog = (rand()%4);
		sleep(numLog);
		fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem2); //wait for semaphore
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
                wait(1); //wait 1 second before writing to the file
		fprintf(file1, "n / log(n) computation: PID: %d     Index: %d     Size: %d \n", getpid(), index, count);
		wait(1); //wait one second before leaving the critical section
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem2); //signal the semaphore
	}
	
        shmdt((void *) intShared); //detach shared memory

return 0;
}
