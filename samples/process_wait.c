#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_WORKERS 10

void initRandomNumbers(unsigned int *numbers, int size);

int main(int argc, char **argv)
{
    pid_t pid[NUM_WORKERS];
    int i;

    printf("Eu sou o Pai %d\n",getpid());
    for(i = 0 ; i < NUM_WORKERS ; i++)
    {
        pid[i] = fork();
        if(pid[i] == -1)
        {
            perror("fork");
            exit(1);
        }
        else if(pid[i] == 0) //filho
        {
            //código do filho aqui
            srand(time(NULL)^getpid());
            unsigned int rnd = rand()%10;
            printf("Sou o filho %d e vou dormir %d seg...\n",getpid(),rnd);
            sleep(rnd);
            printf("Eu %d acordei e vou terminar...\n",getpid());
            exit(1);
        }
        else //pai
        {
            //nada faz aqui
        }
    }

    //pai espera todos os filhos
    while(wait(NULL) > 0);

    printf("Pai %d terminou\n",getpid());
    
    return 0;
}

