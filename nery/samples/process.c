#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void writePipe(int* P){
  fclose(P[0]);
  write(P[1], "1234", 5);
}

void* readPipe(int* P){
  int bytesRead; int bufferSize = 1024;
  void* buffer = createSharedMemoryBlock(bufferSize);
  fclose(P[1]);

  bytesRead = read(P[0], buffer, bufferSize);
  return buffer;
}

int main(int argc, char **argv)
{
    pid_t pid;

    pid = fork();
    if(pid < 0)
    {
        fprintf(stderr,"'fork error', não foi possivel criar processo filho\n");
        exit(1);
    }
    else if(pid == 0) //vai filhão!
    {
        printf(" FILHO: Este é o processo filho!\n");
        printf(" FILHO: Meu PID é: %d\n",getpid());
        printf(" FILHO: O PID do meu pai é: %d\n",getppid());
        printf(" FILHO: Fui!\n");
    }
    else //pai
    {
        printf(" PAI: Este é o processo pai!\n");
        printf(" PAI: Meu PID é: %d\n",getpid());
        printf(" PAI: O PID do meu filho é: %d\n",pid);
        printf(" PAI: Agora vou esperar o meu filho terminar...\n");
        wait(NULL);
        printf(" PAI: Fui!\n");
    }

    return 0;
}
