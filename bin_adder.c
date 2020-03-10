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
        int numbers[65];
};

struct sharedMem *intShared;

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

        int sum = 0;
        FILE* file1;
        file1 = fopen("adder_log", "a");
        int i;
        const time_t tma = time(NULL);
        char * tme = ctime( & tma);
        for (i = 0; i < 1; i++)
        {
                int num = (rand()%4);
                sleep(num);
                fprintf(stderr, "Process: %d attempting to enter critical section at time: %s seconds\n", getpid(), tme);
                sem_wait(sem);
                fprintf(stderr, "Process %d has entered critical section at time: %s seconds\n", getpid(), tme);
                wait(1);
                fprintf(file1, "PID: %d Index: %d Size: %d \n",getpid(), (index + i), intShared->numbers[index + i]);
                wait(1);
                fprintf(stderr, "Process: %d has left the critical section at time: %s seconds\n", getpid(), tme);
                sem_post(sem);
        }
        shmdt((void *) intShared);

return 0;
}
