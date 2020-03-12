#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>

struct sharedMem
{
	int hindex;
	int tindex;
	int eindex;
        int numbers[65];
};

struct sharedMem *intShared;

int sum1 (int n, int A[ ])
{

  if (n == 1)
    return A[0];
  int pairs_sum[n / 2];
  int j = 0;
  int i = 0;
  for (i = 0; i < n; i = i + 2)
    {
      pairs_sum[j] = A[i] + A[i + 1];
      j++;
    }
  return sum1 (n / 2, pairs_sum);
}

int main(int argc, char *argv[])
{

        struct timespec tyme;
        clock_gettime(CLOCK_MONOTONIC, & tyme);
        srand((unsigned)(tyme.tv_sec ^ tyme.tv_nsec ^ (tyme.tv_nsec >> 31)));
        sem_t* sem;
        sem = sem_open("p3sem",0);
        if(sem == SEM_FAILED)
        {
                fprintf(stderr,"./bin_adder: Error in sem_open");
                exit(1);
        }

        int index = atoi(argv[0]);
	int count = atoi(argv[1]);
	int n = atoi(argv[2]);
	
        key_t mem_key = ftok("./master.c", 1);
        int shmid2 = shmget(mem_key, sizeof(struct sharedMem), 0666);
        if(shmid2 == -1)
        {
                printf("Error in bin_adder shmget\n");
                exit(0);
        }

        intShared = shmat(shmid2,(void *)0,0);
        if((intptr_t) intShared == -1)
        {
                printf("Error in bin_adder shmat\n");
                exit(0);
        }

	int i = 0;
        int A[n];
        int size = n;
        for(i=0; i<size; i++)
        {
                A[i] = intShared->numbers[i];
        }

        FILE* file1;
        file1 = fopen("adder_log", "a");
        const time_t tma = time(NULL);
        char * tme = ctime( & tma);
	int hrquest;
	int trquest; 
	intShared->hindex = index;
        intShared->tindex = index + (count / 2);
        intShared->eindex = count;

        for (i = 0, hrquest = intShared->hindex, trquest = intShared->tindex; i < count / 2; i++, hrquest++, trquest++)
	{
		int result = intShared->numbers[hrquest] + intShared->numbers[trquest];	
                int num = (rand()%4);

                sleep(num);
                fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem);
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
		wait(1);
                fprintf(file1, "PID: %d Index: %d Size: %d Values: %d and %d Result: %d\n",getpid(), (index + i), count,intShared->numbers[hrquest], intShared->numbers[trquest],result);
                wait(1);
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem);
		intShared->numbers[hrquest] = result;
        }

        shmdt((void *) intShared);

return 0;
}
