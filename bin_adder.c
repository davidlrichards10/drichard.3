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

	/* Set time */
        const time_t tma = time(NULL);
        char * tme = ctime( & tma);
	
	/* If computation flag is 1 perform n/2 computation */
	if (intShared->computationFlg == 1)
	{
                int num = (rand()%4); // store and create random number between 0 and 3 for sleep call
        	sleep(num);
                fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem);
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
                sleep(1);
		/* Print PID, index, size, time, values and result to adder_log */ 
                fprintf(file1, "                         %d\t\t%d\t\t%d\t\t%d\t\t%d+%d\n%s\n", getpid(), index, count,intShared->numbers[index] + intShared->numbers[index+1], intShared->numbers[index], intShared->numbers[index+1],tme);
                
		int result = intShared->numbers[index];
		int result2 = intShared->numbers[index+1];
		intShared->numbers[index] = result + result2;
		sleep(1);
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem);

		fclose(file1);
	}
	
	/* If computation flag is 0 perform second calculation */
	else
	{
		int numLog = (rand()%4);
		sleep(numLog);
		fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem2); 
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
		sleep(1);
		/* If groups of 2 add numbers accordingly and Print PID, index, size, time, values and result to adder_log */
		if(count == 2)
		{
		fprintf(file1, "                         %d\t\t%d\t\t%d\t\t%d\t\t%d+%d\n%s\n", getpid(), index, count,intShared->numbersLog[index] + intShared->numbersLog[index+1], intShared->numbersLog[index], intShared->numbersLog[index+1],tme);
                int resultLog = intShared->numbersLog[index];
                int result2Log = intShared->numbersLog[index+1];
                intShared->numbersLog[index] = resultLog + result2Log;
		}
		/* If groups of 3 add numbers accordingly and Print PID, index, size, time, values and result to adder_log */
		if(count == 3)
		{
		fprintf(file1, "                         %d\t\t%d\t\t%d\t\t%d\t\t%d+%d+%d\n%s\n", getpid(), index, count,intShared->numbersLog[index] + intShared->numbersLog[index+1] + intShared->numbersLog[index+2], intShared->numbersLog[index], intShared->numbersLog[index+1], intShared->numbersLog[index+2],tme);
		int resultLog = intShared->numbersLog[index];
                int result2Log = intShared->numbersLog[index+1];
		int result3Log = intShared->numbersLog[index+2];
                intShared->numbersLog[index] = resultLog + result2Log + result3Log;
		}
		/* If groups of 4 add numbers accordingly and Print PID, index, size, time, values and result to adder_log */
		if(count == 4)
                {
                fprintf(file1, "                         %d\t\t%d\t\t%d\t\t%d\t\t%d+%d+%d+%d\n%s\n", getpid(), index, count,intShared->numbersLog[index] + intShared->numbersLog[index+1] + intShared->numbersLog[index+2] + intShared->numbersLog[index+3], intShared->numbersLog[index], intShared->numbersLog[index+1], intShared->numbersLog[index+2], intShared->numbersLog[index+3],tme);
                int resultLog = intShared->numbersLog[index];
                int result2Log = intShared->numbersLog[index+1];
                int result3Log = intShared->numbersLog[index+2];
                int result4Log = intShared->numbersLog[index+3];
		intShared->numbersLog[index] = resultLog + result2Log + result3Log + result4Log;
                }
		/* If groups of 5 add numbers accordingly and Print PID, index, size, time, values and result to adder_log */
		if(count == 5)
                {
                fprintf(file1, "                         %d\t\t%d\t\t%d\t\t\%d\t\t%d+%d+%d+%d+%d\n%s\n", getpid(), index, count,intShared->numbersLog[index] + intShared->numbersLog[index+1] + intShared->numbersLog[index+2] + intShared->numbersLog[index+3] + intShared->numbersLog[index+4], intShared->numbersLog[index], intShared->numbersLog[index+1], intShared->numbersLog[index+2], intShared->numbersLog[index+3],intShared->numbersLog[index+4],tme);
                int resultLog = intShared->numbersLog[index];
                int result2Log = intShared->numbersLog[index+1];
                int result3Log = intShared->numbersLog[index+2];
                int result4Log = intShared->numbersLog[index+3];
		int result5Log = intShared->numbersLog[index+4];
		intShared->numbersLog[index] = resultLog + result2Log + result3Log + result4Log + result5Log;
                }
		/* If groups of 6 add numbers accordingly and Print PID, index, size, time, values and result to adder_log */
		if(count == 6)
                {
                fprintf(file1, "                         %d\t\t%d\t\t%d\t\t%d\t\t%d+%d+%d+%d+%d+%d\n%s\n", getpid(), index, count,intShared->numbersLog[index] + intShared->numbersLog[index+1] + intShared->numbersLog[index+2] + intShared->numbersLog[index+3] + intShared->numbersLog[index+4] + intShared->numbersLog[index+5], intShared->numbersLog[index], intShared->numbersLog[index+1], intShared->numbersLog[index+2], intShared->numbersLog[index+3],intShared->numbersLog[index+4],intShared->numbersLog[index+5],tme);
                int resultLog = intShared->numbersLog[index];
                int result2Log = intShared->numbersLog[index+1];
                int result3Log = intShared->numbersLog[index+2];
                int result4Log = intShared->numbersLog[index+3];
                int result5Log = intShared->numbersLog[index+4];
		int result6Log = intShared->numbersLog[index+5];
                intShared->numbersLog[index] = resultLog + result2Log + result3Log + result4Log + result5Log + result6Log;
                }

		sleep(1); //wait one second before leaving the critical section
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem2); //signal the semaphore
	}
	
        shmdt((void *) intShared); //detach shared memory

return 0;
}
