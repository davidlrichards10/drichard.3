/*
 * Date:   March 12, 2020
 * Author: David Richards
 * Class:  CS4760
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

int shmid;
void sigErrors(int signum);
int squarert(int);

struct sharedMem
{
        int numbers[65];
};

struct sharedMem *intShared;

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

void detach()
{
        shmdt((void*)intShared);
        shmctl(shmid, IPC_RMID, NULL);
}

int main(int argc, char* argv[])
{
        int n = 64;
        int c;

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


        setUp();
        FILE *file = fopen("intFile", "w");
        if(!file)
        {
                perror("Error could not open file\n");
                return EXIT_FAILURE;
        }

        srand(time(0));
        int i;
        for(i = 0; i < n; i++)
        {
               int num = (rand() % (256 - 0 + 1)) + 0;
               fprintf(file, "%d\n", num);
	}

	/*fprintf(file, "1\n");
	fprintf(file, "2\n");
	fprintf(file, "3\n");
	fprintf(file, "4\n");
	fprintf(file, "5\n");
	fprintf(file, "6\n");
	fprintf(file, "7\n");
	fprintf(file, "8\n");*/
        fclose(file);

        fopen("intFile", "r");
        if(!file)
        {
                perror("Error opening file in master.c\n");
                return EXIT_FAILURE;
        }

        for(i = 0; i < n; i++)
        {
                fscanf(file, "%d", &intShared->numbers[i]);
                printf("%d\n", intShared->numbers[i]);
        }

        fclose(file);

        if (signal(SIGINT, sigErrors) == SIG_ERR) //sigerror on cntrl-c
        {
                exit(0);
        }

        if (signal(SIGALRM, sigErrors) == SIG_ERR) //sigerror on program exceeding 2 second alarm
        {
                exit(0);
        }

        sem_t* sem;
        sem = sem_open("p3sem", O_CREAT, 0644, 1);
        if(sem == SEM_FAILED)
        {
                fprintf(stderr,"Error in sem_open");
                exit(0);
        }

                alarm(100);
                int status;
                int numbers = n;
                int active = 1;
                int k;
		int count = n;
                pid_t pids[n],wpid;
                while(k < numbers - 1)
                {
			k = 0;
                        if(active < 2)
                        {
                                pids[k] = fork();
                                if(pids[k] == 0)
                                {
                                        char index[20];
					char yy[20];
                                        sprintf(index, "%d", k);
					sprintf(yy, "%d", count);
                                        execl("./bin_adder",index,yy,NULL);
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
			if (count == 1)
			{
				break;
			}
                }

        detach();
        sem_unlink("p3sem");
        return 0;
}

void sigErrors(int signum) //function for signal handling, either 2 second alarm or Ctrl-c
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
